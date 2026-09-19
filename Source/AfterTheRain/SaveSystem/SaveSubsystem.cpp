#include "SaveSystem/SaveSubsystem.h"
#include "SaveSystem/ATRSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Core/StoryFlagSubsystem.h"
#include "Core/CharacterRegistrySubsystem.h"
#include "Quests/QuestSubsystem.h"
#include "Investigation/EvidenceSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Characters/ATRCharacterBase.h"
#include "Characters/RelationshipComponent.h"
#include "Player/ATRPlayerCharacter.h"
#include "AfterTheRain.h"

bool USaveSubsystem::SaveGame(const FString& SlotName, const FString& DisplayName)
{
	UATRSaveGame* SaveGameObject = Cast<UATRSaveGame>(UGameplayStatics::CreateSaveGameObject(UATRSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->SaveSlotDisplayName = DisplayName;
	SaveGameObject->SavedAtUTC = FDateTime::UtcNow();

	if (UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>())
	{
		SaveGameObject->StoryFlags = StoryFlags->GetAllFlags();
	}

	if (UEvidenceSubsystem* Evidence = GetGameInstance()->GetSubsystem<UEvidenceSubsystem>())
	{
		SaveGameObject->DiscoveredEvidenceIDs = Evidence->GetAllDiscovered();
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	if (UQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UQuestSubsystem>())
	{
		SaveGameObject->Quests.Reset();
		for (const FName& QuestID : Quests->GetAllRegisteredQuestIDs())
		{
			FQuestSaveEntry Entry;
			Entry.QuestID = QuestID;
			Entry.State = static_cast<uint8>(Quests->GetQuestState(QuestID));
			Entry.CompletedObjectiveIDs = Quests->GetCompletedObjectives(QuestID);
			SaveGameObject->Quests.Add(Entry);
		}
	}

	if (UCharacterRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UCharacterRegistrySubsystem>())
	{
		SaveGameObject->Relationships.Reset();
		for (const FName& CharacterID : Registry->GetAllRegisteredCharacterIDs())
		{
			AATRCharacterBase* Character = Registry->FindCharacter(CharacterID);

			// The player's own RelationshipComponent (inherited from AATRCharacterBase) is
			// not meaningful state — nobody has a "Trust" value toward themselves — so it's
			// excluded from the save file rather than persisted as always-zero noise.
			if (Character && Character->RelationshipComponent && Cast<APawn>(Character) != PlayerPawn)
			{
				FRelationshipSaveEntry Entry;
				Entry.CharacterID = CharacterID;
				Entry.State = Character->RelationshipComponent->GetFullState();
				SaveGameObject->Relationships.Add(Entry);
			}
		}
	}

	if (PlayerPawn)
	{
		SaveGameObject->PlayerTransform = PlayerPawn->GetActorTransform();

		if (UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>())
		{
			SaveGameObject->InventoryItems = Inventory->GetAllItems();
		}
	}

	SaveGameObject->CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, SlotIndex);
	UE_LOG(LogAfterTheRain, Log, TEXT("SaveGame('%s') -> %s (flags:%d, evidence:%d, quests:%d, relationships:%d, items:%d)"),
		*SlotName, bSuccess ? TEXT("OK") : TEXT("FAILED"),
		SaveGameObject->StoryFlags.Num(), SaveGameObject->DiscoveredEvidenceIDs.Num(),
		SaveGameObject->Quests.Num(), SaveGameObject->Relationships.Num(), SaveGameObject->InventoryItems.Num());

	return bSuccess;
}

bool USaveSubsystem::LoadGame(const FString& SlotName)
{
	if (!DoesSaveExist(SlotName))
	{
		return false;
	}

	UATRSaveGame* SaveGameObject = Cast<UATRSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex));
	if (!SaveGameObject)
	{
		return false;
	}

	MigrateIfNeeded(SaveGameObject);

	// Every restore below replaces state wholesale (SetFlags/RestoreItems/RestoreState/
	// RestoreQuestRuntimeState all overwrite rather than append) specifically so loading
	// the same slot twice, or loading after partial play, never duplicates inventory
	// items, story flags, or relationship/quest progress.

	if (UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>())
	{
		StoryFlags->RestoreFlags(SaveGameObject->StoryFlags);
	}

	if (UEvidenceSubsystem* Evidence = GetGameInstance()->GetSubsystem<UEvidenceSubsystem>())
	{
		Evidence->RestoreDiscovered(SaveGameObject->DiscoveredEvidenceIDs);
	}

	if (UQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UQuestSubsystem>())
	{
		for (const FQuestSaveEntry& Entry : SaveGameObject->Quests)
		{
			Quests->RestoreQuestRuntimeState(Entry.QuestID, static_cast<EQuestState>(Entry.State), Entry.CompletedObjectiveIDs);
		}
	}

	if (UCharacterRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UCharacterRegistrySubsystem>())
	{
		for (const FRelationshipSaveEntry& Entry : SaveGameObject->Relationships)
		{
			if (AATRCharacterBase* Character = Registry->FindCharacter(Entry.CharacterID))
			{
				Character->RelationshipComponent->RestoreState(Entry.State);
			}
			else
			{
				// Expected if the relevant NPC's level/actor isn't currently loaded — the
				// caller is responsible for level-traveling to SaveGameObject->CurrentLevelName
				// BEFORE calling LoadGame so NPCs have already registered themselves.
				UE_LOG(LogAfterTheRain, Warning, TEXT("LoadGame: character '%s' is not currently registered (level not loaded yet?) — relationship state not restored for it."), *Entry.CharacterID.ToString());
			}
		}
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		PlayerPawn->SetActorTransform(SaveGameObject->PlayerTransform);

		if (UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->RestoreItems(SaveGameObject->InventoryItems);
		}
	}

	UE_LOG(LogAfterTheRain, Log, TEXT("LoadGame('%s') -> OK (flags:%d, evidence:%d, quests:%d, relationships:%d, items:%d)"),
		*SlotName, SaveGameObject->StoryFlags.Num(), SaveGameObject->DiscoveredEvidenceIDs.Num(),
		SaveGameObject->Quests.Num(), SaveGameObject->Relationships.Num(), SaveGameObject->InventoryItems.Num());

	return true;
}

bool USaveSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex);
}

TArray<FString> USaveSubsystem::GetAvailableSaveSlots() const
{
	// Platform save systems vary in whether they support slot enumeration; on platforms
	// without it, the project should maintain its own index (e.g. a small JSON/ini file
	// of known slot names) written alongside each SaveGame call. Left as a project-specific
	// extension point rather than assumed here — NOT implemented, tracked in ROADMAP.md.
	UE_LOG(LogAfterTheRain, Log, TEXT("GetAvailableSaveSlots: implement slot enumeration for target platform's save system."));
	return TArray<FString>();
}

void USaveSubsystem::MigrateIfNeeded(UATRSaveGame* SaveGameObject) const
{
	if (!SaveGameObject || SaveGameObject->SaveVersion >= ATR_SAVE_VERSION_CURRENT)
	{
		return;
	}

	// Example migration ladder:
	// if (SaveGameObject->SaveVersion < 2) { /* upgrade v1 -> v2 fields */ }
	SaveGameObject->SaveVersion = ATR_SAVE_VERSION_CURRENT;
}
