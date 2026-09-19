#include "Dialogue/DialogueSubsystem.h"
#include "Characters/ATRCharacterBase.h"
#include "Characters/RelationshipComponent.h"
#include "Core/StoryFlagSubsystem.h"
#include "Core/CharacterRegistrySubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Quests/QuestSubsystem.h"
#include "Cinematics/CinematicSubsystem.h"
#include "Camera/ATRDialogueCameraComponent.h"
#include "Player/ATRPlayerController.h"
#include "GameFramework/Controller.h"
#include "AfterTheRain.h"

bool UDialogueSubsystem::StartConversation(UATRDialogueData* DialogueData, AATRCharacterBase* Speaker, AATRCharacterBase* Listener)
{
	if (!DialogueData || !DialogueData->NodeTable)
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("StartConversation: invalid DialogueData or missing NodeTable."));
		return false;
	}

	if (IsConversationActive())
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("StartConversation: a conversation is already active; call EndConversation first."));
		return false;
	}

	if (!EvaluateCondition(DialogueData->AvailabilityCondition, Speaker, Listener))
	{
		return false;
	}

	ActiveData = DialogueData;
	ActiveSpeaker = Speaker;
	ActiveListener = Listener;

	// Auto-set a "Met<CharacterID>" story flag the first time a conversation with this
	// speaker begins. This is the "MetSarah"-style flag the story-flag system needs, but
	// derived generically from CharacterID so it works for Daniel/Maya/any future NPC
	// without any per-character code.
	if (Speaker && !Speaker->GetCharacterID().IsNone())
	{
		if (UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>())
		{
			const FName MetFlag(*FString::Printf(TEXT("Met%s"), *Speaker->GetCharacterID().ToString()));
			StoryFlags->SetFlag(MetFlag, true);
		}
	}

	SetDialoguePresentationActive(true);

	UE_LOG(LogAfterTheRain, Log, TEXT("Dialogue Started (Speaker: %s)"), Speaker ? *Speaker->GetCharacterID().ToString() : TEXT("<none>"));
	OnDialogueStarted.Broadcast(Speaker, Listener);

	ShowNode(DialogueData->EntryNodeID);
	return true;
}

void UDialogueSubsystem::ShowNode(FName NodeID)
{
	if (!ActiveData || !ActiveData->NodeTable)
	{
		return;
	}

	static const FString ContextString(TEXT("DialogueNodeLookup"));
	const FDialogueNode* Row = ActiveData->NodeTable->FindRow<FDialogueNode>(NodeID, ContextString);

	if (!Row)
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("ShowNode: NodeID '%s' not found. Ending conversation."), *NodeID.ToString());
		EndConversation();
		return;
	}

	if (!EvaluateCondition(Row->EntryCondition, ActiveSpeaker, ActiveListener))
	{
		// A node the player can't currently reach should not be a dead end for writers:
		// fall through to its auto-advance target so branches can gracefully skip content.
		// (This skip is silent by design: no text was shown, so there's nothing to "read".)
		if (!Row->AutoAdvanceNodeID.IsNone())
		{
			ShowNode(Row->AutoAdvanceNodeID);
		}
		else
		{
			EndConversation();
		}
		return;
	}

	CurrentNodeID = NodeID;
	CachedAutoAdvanceNodeID = Row->AutoAdvanceNodeID;

	CachedAvailableResponses.Reset();
	for (const FDialogueResponse& Response : Row->Responses)
	{
		if (EvaluateCondition(Response.Condition, ActiveSpeaker, ActiveListener))
		{
			CachedAvailableResponses.Add(Response);
		}
	}

	UE_LOG(LogAfterTheRain, Log, TEXT("Dialogue Node: %s (Speaker: %s)"), *NodeID.ToString(), *Row->SpeakerCharacterID.ToString());
	OnDialogueNodeShown.Broadcast(*Row);

	// IMPORTANT: this node is now shown and stays shown. Nodes with no responses (pure
	// narration OR a true ending like "Node_End") are NOT auto-chained here anymore — that
	// used to skip the player past text before they could read it (including the final
	// line of a conversation, which never got a chance to display). The UI/input layer
	// calls ContinueDialogue() when the player is ready to move on; see its comment for
	// what happens next.
}

void UDialogueSubsystem::ContinueDialogue()
{
	if (!IsConversationActive())
	{
		return;
	}

	if (CachedAvailableResponses.Num() > 0)
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("ContinueDialogue: current node has selectable responses; call SelectResponse(Index) instead."));
		return;
	}

	if (!CachedAutoAdvanceNodeID.IsNone())
	{
		ShowNode(CachedAutoAdvanceNodeID);
	}
	else
	{
		// No responses and nowhere to auto-advance to: this IS the end of the conversation.
		// The player has already seen this node's text via OnDialogueNodeShown and is now
		// explicitly dismissing it, so it's safe to tear down here.
		EndConversation();
	}
}

void UDialogueSubsystem::SelectResponse(int32 ResponseIndex)
{
	if (!IsConversationActive())
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("SelectResponse: no conversation is active."));
		return;
	}

	if (!CachedAvailableResponses.IsValidIndex(ResponseIndex))
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("SelectResponse: index %d out of range."), ResponseIndex);
		return;
	}

	const FDialogueResponse Response = CachedAvailableResponses[ResponseIndex];

	// Clear the response list immediately, before applying consequences or moving nodes.
	// If SelectResponse were somehow re-entered with the same index (e.g. a UI double-fire
	// before the first call returns), IsValidIndex above would now fail against an empty
	// array, so a consequence can never be applied twice for one player choice.
	CachedAvailableResponses.Reset();

	UE_LOG(LogAfterTheRain, Log, TEXT("Dialogue Choice: %s"), *Response.ResponseText.ToString());

	ApplyConsequence(Response.Consequence);

	if (Response.Consequence.NextNodeID.IsNone())
	{
		EndConversation();
	}
	else
	{
		ShowNode(Response.Consequence.NextNodeID);
	}
}

void UDialogueSubsystem::ApplyConsequence(const FDialogueConsequence& Consequence)
{
	if (UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>())
	{
		for (const FName& Flag : Consequence.StoryFlagsToSet)
		{
			StoryFlags->SetFlag(Flag, true);
			UE_LOG(LogAfterTheRain, Log, TEXT("Story Flag: %s = true"), *Flag.ToString());
		}
		for (const FName& Flag : Consequence.StoryFlagsToClear)
		{
			StoryFlags->ClearFlag(Flag);
			UE_LOG(LogAfterTheRain, Log, TEXT("Story Flag: %s cleared"), *Flag.ToString());
		}
	}

	// RELATIONSHIP OWNERSHIP: a relationship effect defaults to modifying the SPEAKER's
	// RelationshipComponent (i.e. the NPC's opinion of the player), not the listener's —
	// the listener is conventionally the player, and the player does not have an opinion
	// of themselves. An effect can still target any other registered character explicitly
	// via TargetCharacterID (see FRelationshipEffect) for the rare case a choice affects
	// someone other than whoever is currently speaking. See ARCHITECTURE.md.
	if (Consequence.RelationshipEffects.Num() > 0)
	{
		UCharacterRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UCharacterRegistrySubsystem>();

		for (const FRelationshipEffect& Effect : Consequence.RelationshipEffects)
		{
			AATRCharacterBase* TargetCharacter = nullptr;

			if (!Effect.TargetCharacterID.IsNone())
			{
				TargetCharacter = Registry ? Registry->FindCharacter(Effect.TargetCharacterID) : nullptr;
				if (!TargetCharacter)
				{
					UE_LOG(LogAfterTheRain, Warning, TEXT("Dialogue consequence: relationship TargetCharacterID '%s' is not a currently registered character — effect skipped."), *Effect.TargetCharacterID.ToString());
					continue;
				}
			}
			else
			{
				TargetCharacter = ActiveSpeaker;
			}

			if (!TargetCharacter || !TargetCharacter->RelationshipComponent)
			{
				UE_LOG(LogAfterTheRain, Warning, TEXT("Dialogue consequence: no valid relationship target/component available (target CharacterID: '%s') — effect skipped."),
					Effect.TargetCharacterID.IsNone() ? TEXT("<speaker>") : *Effect.TargetCharacterID.ToString());
				continue;
			}

			UE_LOG(LogAfterTheRain, Log, TEXT("Relationship Target: %s"), *TargetCharacter->GetCharacterID().ToString());
			TargetCharacter->RelationshipComponent->ModifyValue(Effect.Axis, Effect.Delta);
		}
	}

	if (UQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UQuestSubsystem>())
	{
		for (const FName& EventName : Consequence.QuestEventsToFire)
		{
			Quests->HandleStoryEvent(EventName);
		}
	}

	if (!Consequence.CinematicToPlay.IsNone())
	{
		if (UCinematicSubsystem* CinematicSubsystem = GetGameInstance()->GetSubsystem<UCinematicSubsystem>())
		{
			CinematicSubsystem->PlayCinematicByID(Consequence.CinematicToPlay);
		}
	}
}

void UDialogueSubsystem::EndConversation()
{
	if (!ActiveData)
	{
		// Already ended (or never started) — avoid a redundant OnDialogueEnded broadcast,
		// which could otherwise fire from both ContinueDialogue() and a UI "Close" handler
		// both reacting to the same final node.
		return;
	}

	SetDialoguePresentationActive(false);

	ActiveData = nullptr;
	ActiveSpeaker = nullptr;
	ActiveListener = nullptr;
	CurrentNodeID = NAME_None;
	CachedAutoAdvanceNodeID = NAME_None;
	CachedAvailableResponses.Reset();

	UE_LOG(LogAfterTheRain, Log, TEXT("Dialogue Ended"));
	OnDialogueEnded.Broadcast();
}

void UDialogueSubsystem::SetDialoguePresentationActive(bool bActive) const
{
	if (!ActiveListener)
	{
		return;
	}

	if (AATRPlayerController* ATRController = Cast<AATRPlayerController>(ActiveListener->GetController()))
	{
		ATRController->SetCinematicModeActive(bActive);
	}
	// NOTE: if ActiveListener is ever an NPC rather than the player (e.g. a future
	// NPC-to-NPC scripted scene the player only watches), GetController() here will not
	// be a player controller and this is correctly a no-op — input restoration only
	// applies to whichever participant is actually player-controlled.

	if (UATRDialogueCameraComponent* DialogueCamera = ActiveListener->FindComponentByClass<UATRDialogueCameraComponent>())
	{
		if (bActive)
		{
			DialogueCamera->BlendToShot(EDialogueShotType::Wide);
		}
		else
		{
			DialogueCamera->BlendBackToGameplay();
		}
	}
}

bool UDialogueSubsystem::EvaluateCondition(const FDialogueCondition& Condition, AATRCharacterBase* Speaker, AATRCharacterBase* Listener) const
{
	UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>();

	if (StoryFlags)
	{
		if (!StoryFlags->HasAllFlags(Condition.RequiredStoryFlags))
		{
			return false;
		}
		for (const FName& Forbidden : Condition.ForbiddenStoryFlags)
		{
			if (StoryFlags->HasFlag(Forbidden))
			{
				return false;
			}
		}
	}

	if (Condition.RelationshipRequirements.Num() > 0)
	{
		// Same ownership rule as FRelationshipEffect: a requirement with no explicit
		// TargetCharacterID checks the SPEAKER's relationship toward the player (e.g.
		// "Sarah's Trust >= 70"), not the listener/player's own component. This is what
		// makes relationship values set via ApplyConsequence() actually readable by later
		// conditions -- both sides of the loop must agree on whose value is being read.
		UCharacterRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UCharacterRegistrySubsystem>();

		for (const FRelationshipRequirement& Req : Condition.RelationshipRequirements)
		{
			AATRCharacterBase* Target = Req.TargetCharacterID.IsNone()
				? Speaker
				: (Registry ? Registry->FindCharacter(Req.TargetCharacterID) : nullptr);

			if (!Target || !Target->RelationshipComponent)
			{
				// Fail closed: an unresolvable relationship target must not silently pass
				// a gate meant to restrict content.
				return false;
			}

			if (!Target->RelationshipComponent->MeetsThreshold(Req.Axis, Req.MinimumValue))
			{
				return false;
			}
		}
	}

	if (Listener)
	{
		if (UInventoryComponent* Inventory = Listener->FindComponentByClass<UInventoryComponent>())
		{
			for (const FName& ItemID : Condition.RequiredItemIDs)
			{
				if (!Inventory->HasItem(ItemID))
				{
					return false;
				}
			}
		}
		else if (Condition.RequiredItemIDs.Num() > 0)
		{
			return false;
		}
	}

	if (UQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UQuestSubsystem>())
	{
		for (const FName& QuestID : Condition.RequiredQuestIDs)
		{
			if (!Quests->IsQuestKnown(QuestID))
			{
				return false;
			}
		}
	}

	return true;
}
