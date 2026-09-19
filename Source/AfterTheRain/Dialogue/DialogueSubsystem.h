#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Dialogue/DialogueTypes.h"
#include "DialogueSubsystem.generated.h"

class AATRCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeShown, const FDialogueNode&, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, AATRCharacterBase*, Speaker, AATRCharacterBase*, Listener);

/**
 * Central runtime driver for conversations. Owns no story content itself — all content
 * lives in UATRDialogueData/DataTable assets so writers can add dialogue without
 * touching C++. This subsystem:
 *   1. Evaluates FDialogueCondition against StoryFlagSubsystem / RelationshipComponent /
 *      InventoryComponent / QuestSubsystem.
 *   2. Applies FDialogueConsequence when a response is chosen.
 *   3. Broadcasts node changes so the Dialogue UI (UMG) can render text/responses.
 *
 * Only one conversation is active at a time (StartConversation asserts none is active).
 */
UCLASS()
class AFTERTHERAIN_API UDialogueSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartConversation(UATRDialogueData* DialogueData, AATRCharacterBase* Speaker, AATRCharacterBase* Listener);

	/** Selects a response by index from the currently shown node's filtered response list. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectResponse(int32 ResponseIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndConversation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool IsConversationActive() const { return ActiveData != nullptr; }

	/** Responses from the current node that currently pass their FDialogueCondition. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	TArray<FDialogueResponse> GetAvailableResponses() const { return CachedAvailableResponses; }

	/**
	 * True when the current node has nothing for the player to choose (no passing
	 * responses) — i.e. it's either a pure narration beat (has an AutoAdvanceNodeID) or the
	 * end of the conversation (does not). The Dialogue UI should show a single "Continue" /
	 * "Close" control bound to ContinueDialogue() whenever this is true, instead of a
	 * response list.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool IsCurrentNodeFinal() const { return CachedAvailableResponses.Num() == 0; }

	/** The row name of the node currently being shown, or NAME_None if no conversation is active. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	FName GetCurrentNodeID() const { return CurrentNodeID; }

	/** The current conversation's speaker/listener — mainly for debug UI; may be null. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	AATRCharacterBase* GetActiveSpeaker() const { return ActiveSpeaker; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	AATRCharacterBase* GetActiveListener() const { return ActiveListener; }

	/**
	 * Called by the UI (or input) when the player is done reading a no-choice node.
	 * Advances to that node's AutoAdvanceNodeID if set, otherwise ends the conversation.
	 * This is what makes an ending node (e.g. "Node_End": no responses, no auto-advance)
	 * terminate correctly instead of leaving the conversation open indefinitely — the line
	 * is shown via OnDialogueNodeShown exactly once, the player reads it at their own pace,
	 * then explicitly dismisses it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ContinueDialogue();

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueNodeShown OnDialogueNodeShown;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	/**
	 * Broadcast once, right as a conversation begins, before the entry node is shown —
	 * this is the UI layer's cue to CREATE/SHOW the dialogue widget (a widget that only
	 * listens to OnDialogueNodeShown would have nothing to display itself with; this is
	 * the signal to appear in the first place). OnDialogueNodeShown then handles the
	 * actual content for every node after that, including the first one.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	/** Evaluates an FDialogueCondition against current game state. Speaker is required to
	 *  resolve the default relationship target (see FRelationshipRequirement); pass the
	 *  NPC whose conversation/response is being gated, even outside an active conversation
	 *  (e.g. UATRDialogueComponent checking availability). */
	bool EvaluateCondition(const FDialogueCondition& Condition, AATRCharacterBase* Speaker, AATRCharacterBase* Listener) const;

private:
	void ShowNode(FName NodeID);
	void ApplyConsequence(const FDialogueConsequence& Consequence);

	/** Enables/disables cinematic input mode and blends the dialogue camera in/out for ActiveListener. */
	void SetDialoguePresentationActive(bool bActive) const;

	UPROPERTY()
	TObjectPtr<UATRDialogueData> ActiveData;

	UPROPERTY()
	TObjectPtr<AATRCharacterBase> ActiveSpeaker;

	UPROPERTY()
	TObjectPtr<AATRCharacterBase> ActiveListener; // typically the player

	FName CurrentNodeID;

	/** Cached from the current row so ContinueDialogue() doesn't need a redundant table lookup. */
	FName CachedAutoAdvanceNodeID;

	UPROPERTY()
	TArray<FDialogueResponse> CachedAvailableResponses;
};
