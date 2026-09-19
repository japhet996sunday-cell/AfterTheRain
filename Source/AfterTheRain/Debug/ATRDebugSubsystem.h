#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ATRDebugSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogATRDialogue, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogATRRelationship, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogATRQuest, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogATRSave, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogATRCinematic, Log, All);

/**
 * Central developer inspection point: dumps current story flags / quest states / evidence /
 * inventory / current dialogue node to the log or an on-screen debug HUD. Entirely
 * compiled out of Shipping builds via WITH_EDITOR / UE_BUILD_SHIPPING guards so none of
 * this ships to players. Bind to a console command (e.g. "ATR.DumpState") in a project-
 * specific DeveloperSettings or exec function for quick access during playtesting.
 */
UCLASS()
class AFTERTHERAIN_API UATRDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
#if !UE_BUILD_SHIPPING
	UFUNCTION(Exec, Category = "Debug")
	void ATR_DumpStoryFlags();

	UFUNCTION(Exec, Category = "Debug")
	void ATR_DumpQuests();

	UFUNCTION(Exec, Category = "Debug")
	void ATR_DumpEvidence();

	UFUNCTION(Exec, Category = "Debug")
	void ATR_DumpInventory();

	UFUNCTION(Exec, Category = "Debug")
	void ATR_DumpRelationships();

	UFUNCTION(Exec, Category = "Debug")
	void ATR_TestSave(const FString& SlotName);

	UFUNCTION(Exec, Category = "Debug")
	void ATR_TestLoad(const FString& SlotName);
#endif
};
