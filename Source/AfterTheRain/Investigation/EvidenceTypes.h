#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EvidenceTypes.generated.h"

UENUM(BlueprintType)
enum class EEvidenceType : uint8
{
	Photograph,
	Message,
	Document,
	Object,
	WitnessStatement,
	Recording,
	Other
};

/**
 * Design-time definition of a discoverable clue. Discovery *state* (has the player found
 * this yet) lives in UEvidenceSubsystem so the same asset can be safely shared/reused;
 * this struct is the display + story-linkage data an evidence-board UI reads.
 */
UCLASS(BlueprintType)
class AFTERTHERAIN_API UATREvidenceData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	FName EvidenceID; // e.g. PHOTO_001

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	EEvidenceType EvidenceType = EEvidenceType::Object;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	TArray<FName> RelatedCharacterIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	FName RelatedLocationID;

	/** Story flag set automatically the moment this is discovered (optional). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evidence")
	FName StoryFlagOnDiscovery;
};
