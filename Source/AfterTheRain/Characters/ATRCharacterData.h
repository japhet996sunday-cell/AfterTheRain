#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ATRCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterTraitEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traits")
	FName TraitName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traits")
	FText Description;
};

/**
 * Static/design-time identity data for a character (protagonist or NPC): the parts that
 * do not change at runtime. Runtime state (relationship values, story flags, current
 * location, memories) lives on the character's components/subsystems, not here — this
 * asset can be freely referenced (e.g. by dialogue UI for a display name/portrait)
 * without exposing mutable game state to content packages.
 *
 * Writers/designers create one Data Asset per character in Content/Data/Characters/.
 */
UCLASS(BlueprintType)
class AFTERTHERAIN_API UATRCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText AgeCategory; // e.g. "Young Adult" — deliberately descriptive, not a raw number

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSoftObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	TSoftClassPtr<APawn> CharacterPawnClass; // Placeholder mesh pawn now; swap for MetaHuman-backed pawn later

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traits")
	TArray<FCharacterTraitEntry> PersonalityTraits;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	TArray<FName> Affiliations;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
	FText DesignNotes;
};
