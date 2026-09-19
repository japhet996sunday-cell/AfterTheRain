#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

class UATRSaveGame;

/**
 * Orchestrates writing/reading UATRSaveGame slots. Gathers state FROM other subsystems
 * (StoryFlagSubsystem, QuestSubsystem, EvidenceSubsystem) and the player's components
 * rather than owning that state itself, so each system remains independently testable.
 * Multiple slots are supported via SlotName; SlotIndex is reserved for platform save
 * systems that require it (e.g. some console certification requirements) and defaults to 0.
 */
UCLASS()
class AFTERTHERAIN_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(const FString& SlotName, const FString& DisplayName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save")
	bool DoesSaveExist(const FString& SlotName) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	TArray<FString> GetAvailableSaveSlots() const;

private:
	/** Applies schema migrations if SaveGame->SaveVersion is older than ATR_SAVE_VERSION_CURRENT. */
	void MigrateIfNeeded(UATRSaveGame* SaveGameObject) const;

	static const int32 SlotIndex = 0;
};
