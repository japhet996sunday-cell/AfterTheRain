#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryTypes.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Generic,
	KeyItem,
	Evidence,
	Collectible,
	Gift,
	QuestItem
};

/** Design-time definition of an item. Runtime inventory only stores ItemID + count/metadata. */
UCLASS(BlueprintType)
class AFTERTHERAIN_API UATRItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bStoryRelevant = false;

	/** Free-form key/value pairs for one-off metadata without needing a new field per item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TMap<FName, FString> Metadata;
};

USTRUCT(BlueprintType)
struct FInventoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FName ItemID;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Count = 1;
};
