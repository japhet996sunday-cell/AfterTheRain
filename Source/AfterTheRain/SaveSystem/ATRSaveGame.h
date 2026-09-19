#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Characters/RelationshipComponent.h"
#include "Inventory/InventoryTypes.h"
#include "ATRSaveGame.generated.h"

/** Bump this whenever the save schema changes; SaveSubsystem::LoadGame migrates from older versions. */
#define ATR_SAVE_VERSION_CURRENT 1

USTRUCT()
struct FRelationshipSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName CharacterID;

	UPROPERTY()
	FRelationshipState State;
};

USTRUCT()
struct FQuestSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName QuestID;

	UPROPERTY()
	uint8 State = 0; // EQuestState

	UPROPERTY()
	TArray<FName> CompletedObjectiveIDs;
};

/**
 * Flat, versioned snapshot of everything needed to resume the game. Deliberately POD-like
 * (no UObject references) so it serializes cleanly across engine versions. Each subsystem
 * exposes GetAll.../RestoreAll... methods (see StoryFlagSubsystem, QuestSubsystem, etc.)
 * that USaveSubsystem reads from / writes into these fields — subsystems never serialize
 * themselves directly, keeping save format changes isolated to this one file.
 */
UCLASS()
class AFTERTHERAIN_API UATRSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = ATR_SAVE_VERSION_CURRENT;

	UPROPERTY()
	FString SaveSlotDisplayName;

	UPROPERTY()
	FDateTime SavedAtUTC;

	UPROPERTY()
	FName CurrentChapterID;

	UPROPERTY()
	FName CurrentLocationID;

	UPROPERTY()
	FString CurrentLevelName;

	UPROPERTY()
	FTransform PlayerTransform;

	UPROPERTY()
	TMap<FName, bool> StoryFlags;

	UPROPERTY()
	TArray<FRelationshipSaveEntry> Relationships;

	UPROPERTY()
	TArray<FInventoryEntry> InventoryItems;

	UPROPERTY()
	TArray<FName> DiscoveredEvidenceIDs;

	UPROPERTY()
	TArray<FQuestSaveEntry> Quests;
};
