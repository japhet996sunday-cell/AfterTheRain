#include "Characters/RelationshipComponent.h"
#include "AfterTheRain.h"

FString LexToString(ERelationshipAxis Axis)
{
	switch (Axis)
	{
	case ERelationshipAxis::Trust:      return TEXT("Trust");
	case ERelationshipAxis::Romance:    return TEXT("Romance");
	case ERelationshipAxis::Friendship: return TEXT("Friendship");
	case ERelationshipAxis::Respect:    return TEXT("Respect");
	case ERelationshipAxis::Suspicion:  return TEXT("Suspicion");
	case ERelationshipAxis::Conflict:   return TEXT("Conflict");
	case ERelationshipAxis::Affection:  return TEXT("Affection");
	default:                            return TEXT("Unknown");
	}
}

URelationshipComponent::URelationshipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URelationshipComponent::ModifyValue(ERelationshipAxis Axis, float Delta)
{
	SetValue(Axis, GetValue(Axis) + Delta);
}

void URelationshipComponent::SetValue(ERelationshipAxis Axis, float NewValue)
{
	const float OldValue = GetValue(Axis);
	const float Clamped = FMath::Clamp(NewValue, MinValue, MaxValue);

	if (!FMath::IsNearlyEqual(OldValue, Clamped))
	{
		State.AxisValues.Add(Axis, Clamped);
		OnAxisChanged.Broadcast(Axis, OldValue, Clamped);

		const FString OwnerName = GetOwner() ? GetOwner()->GetName() : TEXT("Unknown");
		UE_LOG(LogAfterTheRain, Log, TEXT("[%s] %s: %.1f -> %.1f"), *OwnerName, *LexToString(Axis), OldValue, Clamped);
	}
}
