#include "Inventory/InventoryComponent.h"

void UInventoryComponent::AddItem(FName ItemID, int32 Count)
{
	for (FInventoryEntry& Entry : Items)
	{
		if (Entry.ItemID == ItemID)
		{
			Entry.Count += Count;
			OnInventoryChanged.Broadcast(ItemID);
			return;
		}
	}

	FInventoryEntry NewEntry;
	NewEntry.ItemID = ItemID;
	NewEntry.Count = Count;
	Items.Add(NewEntry);
	OnInventoryChanged.Broadcast(ItemID);
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (Items[Index].ItemID == ItemID)
		{
			if (Items[Index].Count < Count)
			{
				return false;
			}

			Items[Index].Count -= Count;
			if (Items[Index].Count <= 0)
			{
				Items.RemoveAt(Index);
			}
			OnInventoryChanged.Broadcast(ItemID);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasItem(FName ItemID) const
{
	return GetItemCount(ItemID) > 0;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	for (const FInventoryEntry& Entry : Items)
	{
		if (Entry.ItemID == ItemID)
		{
			return Entry.Count;
		}
	}
	return 0;
}
