#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, FName, ItemID);

/**
 * Player-side (or any actor) inventory of item IDs + counts. Item definitions are looked
 * up from UATRItemData assets by ID elsewhere (UI, dialogue conditions); this component
 * only tracks *possession*, keeping save data small and decoupled from content assets.
 */
UCLASS(ClassGroup = (AfterTheRain), meta = (BlueprintSpawnableComponent))
class AFTERTHERAIN_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItem(FName ItemID, int32 Count = 1);

	/** Returns false if the item isn't present in sufficient quantity. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	TArray<FInventoryEntry> GetAllItems() const { return Items; }

	void RestoreItems(const TArray<FInventoryEntry>& InItems) { Items = InItems; }

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryEntry> Items;
};
