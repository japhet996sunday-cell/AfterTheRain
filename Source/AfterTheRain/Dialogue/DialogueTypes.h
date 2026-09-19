#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Characters/RelationshipComponent.h"
#include "DialogueTypes.generated.h"

/** A single "cost/effect" pair reused for both gating conditions and consequences. */
USTRUCT(BlueprintType)
struct FRelationshipRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	ERelationshipAxis Axis = ERelationshipAxis::Trust;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	float MinimumValue = 0.f;

	/** Whose relationship this checks. NAME_None (default) = the conversation's speaker —
	 *  mirrors FRelationshipEffect::TargetCharacterID so conditions and consequences use
	 *  the same targeting rule and stay consistent with each other. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FName TargetCharacterID;
};

USTRUCT(BlueprintType)
struct FRelationshipEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	ERelationshipAxis Axis = ERelationshipAxis::Trust;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float Delta = 0.f;

	/**
	 * Whose relationship state this effect modifies. Left as NAME_None (the default) for
	 * the common case: the effect applies to the current conversation's SPEAKER (i.e. the
	 * NPC's opinion of the player changes — "Sarah's Trust in the protagonist +10").
	 * Set explicitly (to any registered CharacterID) only when a scene needs to affect a
	 * character other than whoever is currently speaking, e.g. a choice made in front of
	 * Sarah that also changes how a third character, referenced but not present, feels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	FName TargetCharacterID;
};

/** Conditions gating whether a dialogue node or a specific response is available. */
USTRUCT(BlueprintType)
struct FDialogueCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	TArray<FName> RequiredStoryFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	TArray<FName> ForbiddenStoryFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	TArray<FRelationshipRequirement> RelationshipRequirements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	TArray<FName> RequiredItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	TArray<FName> RequiredQuestIDs; // quests that must be active or complete (see QuestSubsystem::IsQuestKnown)
};

/** What happens when a response is chosen: relationship/flag/quest changes + optional jumps. */
USTRUCT(BlueprintType)
struct FDialogueConsequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FRelationshipEffect> RelationshipEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FName> StoryFlagsToSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FName> StoryFlagsToClear;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	TArray<FName> QuestEventsToFire; // forwarded to QuestSubsystem::HandleStoryEvent

	/** Optional: cinematic to play (via CinematicTriggerActor lookup by ID) before continuing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consequence")
	FName CinematicToPlay;

	/** Next node ID within the same DialogueData asset. "" / NAME_None ends the conversation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	FName NextNodeID;
};

USTRUCT(BlueprintType)
struct FDialogueResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response")
	FText ResponseText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response")
	FDialogueCondition Condition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Response")
	FDialogueConsequence Consequence;
};

USTRUCT(BlueprintType)
struct FDialogueNode : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	FName NodeID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	FName SpeakerCharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	FText Text;

	/** Gate on whether this node itself can be reached (checked before showing it). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	FDialogueCondition EntryCondition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
	TArray<FDialogueResponse> Responses;

	/** If Responses is empty, the node auto-advances to this node after Text is shown (or ends if none). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	FName AutoAdvanceNodeID;
};

/** A full conversation: a DataTable of FDialogueNode rows plus the ID of the entry node. */
UCLASS(BlueprintType)
class AFTERTHERAIN_API UATRDialogueData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName ConversationID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<class UDataTable> NodeTable; // rows of FDialogueNode

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName EntryNodeID;

	/** Story flags that must ALL be true for this conversation to be offered at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FDialogueCondition AvailabilityCondition;
};
