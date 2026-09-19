#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestTypes.generated.h"

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Unknown,    // never started, not yet even hidden-visible
	Hidden,     // exists, tracked internally, not shown to the player
	Active,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class EQuestCategory : uint8
{
	MainStory,
	Optional,
	Relationship,
	Investigation
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FText Description;

	/** Story event name (fired from dialogue/evidence/triggers) that completes this objective. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FName CompletionEventName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool bHiddenUntilActive = false;
};

/**
 * Design-time quest definition. Runtime progress (current state, which objectives are
 * complete) lives in UQuestSubsystem keyed by QuestID, so this asset is safely shareable.
 */
UCLASS(BlueprintType)
class AFTERTHERAIN_API UATRQuestData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	EQuestCategory Category = EQuestCategory::MainStory;

	/** Story event name that starts this quest when fired. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FName StartEventName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestObjective> Objectives;

	/** If true, the quest exists (tracked, objectives can complete) before it's shown in the quest log. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bStartsHidden = false;
};
