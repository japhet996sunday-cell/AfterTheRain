#include "Debug/ATRDebugSubsystem.h"
#include "Core/StoryFlagSubsystem.h"
#include "Core/CharacterRegistrySubsystem.h"
#include "Quests/QuestSubsystem.h"
#include "Investigation/EvidenceSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Characters/ATRCharacterBase.h"
#include "Characters/RelationshipComponent.h"
#include "SaveSystem/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogATRDialogue);
DEFINE_LOG_CATEGORY(LogATRRelationship);
DEFINE_LOG_CATEGORY(LogATRQuest);
DEFINE_LOG_CATEGORY(LogATRSave);
DEFINE_LOG_CATEGORY(LogATRCinematic);

#if !UE_BUILD_SHIPPING

void UATRDebugSubsystem::ATR_DumpStoryFlags()
{
	if (UStoryFlagSubsystem* StoryFlags = GetGameInstance()->GetSubsystem<UStoryFlagSubsystem>())
	{
		for (const auto& Pair : StoryFlags->GetAllFlags())
		{
			UE_LOG(LogAfterTheRain, Display, TEXT("Flag: %s = %s"), *Pair.Key.ToString(), Pair.Value ? TEXT("true") : TEXT("false"));
		}
	}
}

void UATRDebugSubsystem::ATR_DumpQuests()
{
	if (UQuestSubsystem* Quests = GetGameInstance()->GetSubsystem<UQuestSubsystem>())
	{
		for (const FName& QuestID : Quests->GetAllRegisteredQuestIDs())
		{
			UE_LOG(LogAfterTheRain, Display, TEXT("Quest: %s = %d (completed objectives: %d)"),
				*QuestID.ToString(), (int32)Quests->GetQuestState(QuestID), Quests->GetCompletedObjectives(QuestID).Num());
		}
	}
}

void UATRDebugSubsystem::ATR_DumpEvidence()
{
	if (UEvidenceSubsystem* Evidence = GetGameInstance()->GetSubsystem<UEvidenceSubsystem>())
	{
		for (const FName& ID : Evidence->GetAllDiscovered())
		{
			UE_LOG(LogAfterTheRain, Display, TEXT("Evidence discovered: %s"), *ID.ToString());
		}
	}
}

void UATRDebugSubsystem::ATR_DumpInventory()
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>())
		{
			for (const FInventoryEntry& Entry : Inventory->GetAllItems())
			{
				UE_LOG(LogAfterTheRain, Display, TEXT("Item: %s x%d"), *Entry.ItemID.ToString(), Entry.Count);
			}
		}
	}
}

void UATRDebugSubsystem::ATR_DumpRelationships()
{
	if (UCharacterRegistrySubsystem* Registry = GetGameInstance()->GetSubsystem<UCharacterRegistrySubsystem>())
	{
		for (const FName& CharacterID : Registry->GetAllRegisteredCharacterIDs())
		{
			AATRCharacterBase* Character = Registry->FindCharacter(CharacterID);
			if (Character && Character->RelationshipComponent)
			{
				const FRelationshipState& State = Character->RelationshipComponent->GetFullState();
				for (const auto& Pair : State.AxisValues)
				{
					UE_LOG(LogAfterTheRain, Display, TEXT("%s | %s: %.1f"), *CharacterID.ToString(), *LexToString(Pair.Key), Pair.Value);
				}
			}
		}
	}
}

void UATRDebugSubsystem::ATR_TestSave(const FString& SlotName)
{
	if (USaveSubsystem* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		const bool bSuccess = Save->SaveGame(SlotName, TEXT("Debug Save"));
		UE_LOG(LogAfterTheRain, Display, TEXT("ATR_TestSave('%s') -> %s"), *SlotName, bSuccess ? TEXT("OK") : TEXT("FAILED"));
	}
}

void UATRDebugSubsystem::ATR_TestLoad(const FString& SlotName)
{
	if (USaveSubsystem* Save = GetGameInstance()->GetSubsystem<USaveSubsystem>())
	{
		const bool bSuccess = Save->LoadGame(SlotName);
		UE_LOG(LogAfterTheRain, Display, TEXT("ATR_TestLoad('%s') -> %s"), *SlotName, bSuccess ? TEXT("OK") : TEXT("FAILED"));
	}
}

#endif // !UE_BUILD_SHIPPING
