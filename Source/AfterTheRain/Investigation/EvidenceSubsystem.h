#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EvidenceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEvidenceDiscovered, FName, EvidenceID);

/**
 * Tracks which evidence IDs have been discovered. Feeds an eventual evidence-board UI and
 * gates dialogue/quest conditions (via FDialogueCondition::RequiredItemIDs-style checks,
 * extended here rather than overloading the item inventory since evidence has its own
 * board/relationship metadata that plain items don't need).
 */
UCLASS()
class AFTERTHERAIN_API UEvidenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Investigation")
	void DiscoverEvidence(FName EvidenceID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Investigation")
	bool IsDiscovered(FName EvidenceID) const { return DiscoveredIDs.Contains(EvidenceID); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Investigation")
	TArray<FName> GetAllDiscovered() const { return DiscoveredIDs.Array(); }

	void RestoreDiscovered(const TArray<FName>& InIDs) { DiscoveredIDs = TSet<FName>(InIDs); }

	UPROPERTY(BlueprintAssignable, Category = "Investigation")
	FOnEvidenceDiscovered OnEvidenceDiscovered;

private:
	UPROPERTY()
	TSet<FName> DiscoveredIDs;
};
