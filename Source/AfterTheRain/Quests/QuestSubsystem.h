#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quests/QuestTypes.h"
#include "QuestSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, FName, QuestID, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, QuestID, FName, ObjectiveID);

USTRUCT()
struct FQuestRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	EQuestState State = EQuestState::Unknown;

	UPROPERTY()
	TSet<FName> CompletedObjectiveIDs;
};

/**
 * Drives quest/objective progress. Quests are started/advanced entirely through named
 * "story events" (HandleStoryEvent) fired by dialogue consequences, evidence discovery,
 * or world triggers — this keeps quest logic decoupled from *how* an event happened.
 *
 * Register quest content via RegisterQuestData (typically at GameInstance init from a
 * curated list, e.g. loaded from an asset manager query over UATRQuestData assets).
 */
UCLASS()
class AFTERTHERAIN_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RegisterQuestData(UATRQuestData* QuestData);

	/** Central event dispatch: starts quests whose StartEventName matches, completes matching objectives. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void HandleStoryEvent(FName EventName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	EQuestState GetQuestState(FName QuestID) const;

	/** True if the quest is Active, Completed, or Failed (i.e. has been encountered at all). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	bool IsQuestKnown(FName QuestID) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteObjective(FName QuestID, FName ObjectiveID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetQuestState(FName QuestID, EQuestState NewState);

	/** All quest IDs currently registered this session (via RegisterQuestData). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<FName> GetAllRegisteredQuestIDs() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	TArray<FName> GetCompletedObjectives(FName QuestID) const;

	/**
	 * Restores runtime state for one quest from a save file. Safe to call before or after
	 * RegisterQuestData for the same QuestID — RegisterQuestData only ever sets a *default*
	 * state when none exists yet, so it never clobbers a state restored here, regardless
	 * of which happens first during session bootstrap.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RestoreQuestRuntimeState(FName QuestID, EQuestState State, const TArray<FName>& CompletedObjectiveIDs);

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnObjectiveCompleted OnObjectiveCompleted;

private:
	void EvaluateQuestCompletion(FName QuestID);

	UPROPERTY()
	TMap<FName, TObjectPtr<UATRQuestData>> RegisteredQuests;

	TMap<FName, FQuestRuntimeState> RuntimeStates;
};
