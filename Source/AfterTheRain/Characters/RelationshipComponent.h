#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RelationshipComponent.generated.h"

/**
 * The relationship axes tracked between the player and an NPC (or, later, NPC-to-NPC).
 * Add new axes here as the story requires; every consumer (dialogue conditions, quest
 * gates, UI) reads through GetValue(EAxis)/SetValue rather than named fields, so adding
 * an axis never requires touching those consumers.
 */
UENUM(BlueprintType)
enum class ERelationshipAxis : uint8
{
	Trust,
	Romance,
	Friendship,
	Respect,
	Suspicion,
	Conflict,
	Affection
};

USTRUCT(BlueprintType)
struct FRelationshipState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Relationship")
	TMap<ERelationshipAxis, float> AxisValues;

	float Get(ERelationshipAxis Axis) const
	{
		const float* Found = AxisValues.Find(Axis);
		return Found ? *Found : 0.f;
	}
};

/** Human-readable name for an axis, used only for logging/UI — not a gameplay-affecting mapping. */
AFTERTHERAIN_API FString LexToString(ERelationshipAxis Axis);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRelationshipAxisChanged, ERelationshipAxis, Axis, float, OldValue, float, NewValue);

/**
 * Attach to any NPC actor (via ATRCharacterBase) to give it a relationship state toward
 * the player. Values are clamped to [MinValue, MaxValue] (design default -100..100) and
 * changes are exposed as a delegate so dialogue/animation/behavior can react without
 * polling. Threshold checks (e.g. "Trust >= 70") are done by callers via GetValue() so
 * thresholds stay data-driven in DialogueData/QuestData rather than hard-coded here.
 */
UCLASS(ClassGroup = (AfterTheRain), meta = (BlueprintSpawnableComponent))
class AFTERTHERAIN_API URelationshipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URelationshipComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Relationship")
	float GetValue(ERelationshipAxis Axis) const { return State.Get(Axis); }

	/** Adds Delta to the axis (positive or negative) and clamps to configured bounds. */
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	void ModifyValue(ERelationshipAxis Axis, float Delta);

	/** Sets the axis to an absolute value (used when restoring from a save game). */
	UFUNCTION(BlueprintCallable, Category = "Relationship")
	void SetValue(ERelationshipAxis Axis, float NewValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Relationship")
	bool MeetsThreshold(ERelationshipAxis Axis, float MinimumValue) const { return GetValue(Axis) >= MinimumValue; }

	const FRelationshipState& GetFullState() const { return State; }
	void RestoreState(const FRelationshipState& InState) { State = InState; }

	UPROPERTY(EditDefaultsOnly, Category = "Relationship|Config")
	float MinValue = -100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Relationship|Config")
	float MaxValue = 100.f;

	UPROPERTY(BlueprintAssignable, Category = "Relationship")
	FOnRelationshipAxisChanged OnAxisChanged;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Relationship")
	FRelationshipState State;
};
