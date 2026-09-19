#include "Quests/QuestSubsystem.h"
#include "AfterTheRain.h"

void UQuestSubsystem::RegisterQuestData(UATRQuestData* QuestData)
{
	if (!QuestData || QuestData->QuestID.IsNone())
	{
		return;
	}
	RegisteredQuests.Add(QuestData->QuestID, QuestData);

	if (!RuntimeStates.Contains(QuestData->QuestID))
	{
		FQuestRuntimeState NewState;
		NewState.State = QuestData->bStartsHidden ? EQuestState::Hidden : EQuestState::Unknown;
		RuntimeStates.Add(QuestData->QuestID, NewState);
	}
}

void UQuestSubsystem::HandleStoryEvent(FName EventName)
{
	// Start any registered quest whose StartEventName matches.
	for (const auto& Pair : RegisteredQuests)
	{
		const FName QuestID = Pair.Key;
		UATRQuestData* QuestData = Pair.Value;

		if (QuestData && QuestData->StartEventName == EventName)
		{
			const EQuestState Current = GetQuestState(QuestID);
			if (Current == EQuestState::Unknown || Current == EQuestState::Hidden)
			{
				SetQuestState(QuestID, EQuestState::Active);
			}
		}
	}

	// Complete any active quest's objective whose CompletionEventName matches.
	for (const auto& Pair : RegisteredQuests)
	{
		const FName QuestID = Pair.Key;
		UATRQuestData* QuestData = Pair.Value;
		if (!QuestData || GetQuestState(QuestID) != EQuestState::Active)
		{
			continue;
		}

		for (const FQuestObjective& Objective : QuestData->Objectives)
		{
			if (Objective.CompletionEventName == EventName)
			{
				CompleteObjective(QuestID, Objective.ObjectiveID);
			}
		}
	}
}

EQuestState UQuestSubsystem::GetQuestState(FName QuestID) const
{
	if (const FQuestRuntimeState* State = RuntimeStates.Find(QuestID))
	{
		return State->State;
	}
	return EQuestState::Unknown;
}

bool UQuestSubsystem::IsQuestKnown(FName QuestID) const
{
	const EQuestState State = GetQuestState(QuestID);
	return State == EQuestState::Active || State == EQuestState::Completed || State == EQuestState::Failed;
}

void UQuestSubsystem::CompleteObjective(FName QuestID, FName ObjectiveID)
{
	FQuestRuntimeState& State = RuntimeStates.FindOrAdd(QuestID);
	bool bAlreadyComplete = false;
	State.CompletedObjectiveIDs.Add(ObjectiveID, &bAlreadyComplete);

	if (!bAlreadyComplete)
	{
		OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
		EvaluateQuestCompletion(QuestID);
	}
}

void UQuestSubsystem::EvaluateQuestCompletion(FName QuestID)
{
	UATRQuestData* QuestData = RegisteredQuests.FindRef(QuestID);
	if (!QuestData)
	{
		return;
	}

	const FQuestRuntimeState& State = RuntimeStates.FindOrAdd(QuestID);

	for (const FQuestObjective& Objective : QuestData->Objectives)
	{
		if (!State.CompletedObjectiveIDs.Contains(Objective.ObjectiveID))
		{
			return; // not all objectives complete yet
		}
	}

	SetQuestState(QuestID, EQuestState::Completed);
}

void UQuestSubsystem::SetQuestState(FName QuestID, EQuestState NewState)
{
	FQuestRuntimeState& State = RuntimeStates.FindOrAdd(QuestID);
	if (State.State != NewState)
	{
		State.State = NewState;
		OnQuestStateChanged.Broadcast(QuestID, NewState);
	}
}

TArray<FName> UQuestSubsystem::GetAllRegisteredQuestIDs() const
{
	TArray<FName> IDs;
	RegisteredQuests.GenerateKeyArray(IDs);
	return IDs;
}

TArray<FName> UQuestSubsystem::GetCompletedObjectives(FName QuestID) const
{
	if (const FQuestRuntimeState* State = RuntimeStates.Find(QuestID))
	{
		return State->CompletedObjectiveIDs.Array();
	}
	return TArray<FName>();
}

void UQuestSubsystem::RestoreQuestRuntimeState(FName QuestID, EQuestState State, const TArray<FName>& CompletedObjectiveIDs)
{
	FQuestRuntimeState& RuntimeState = RuntimeStates.FindOrAdd(QuestID);
	RuntimeState.State = State;
	RuntimeState.CompletedObjectiveIDs = TSet<FName>(CompletedObjectiveIDs);
	// Deliberately does not broadcast OnQuestStateChanged/OnObjectiveCompleted: this is a
	// silent restore of prior progress, not a new gameplay event UI should react/animate to.
}
