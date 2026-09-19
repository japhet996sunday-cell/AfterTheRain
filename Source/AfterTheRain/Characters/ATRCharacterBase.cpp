#include "Characters/ATRCharacterBase.h"
#include "Characters/ATRCharacterData.h"
#include "Characters/RelationshipComponent.h"
#include "Dialogue/ATRDialogueComponent.h"
#include "Core/CharacterRegistrySubsystem.h"
#include "Kismet/GameplayStatics.h"

AATRCharacterBase::AATRCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RelationshipComponent = CreateDefaultSubobject<URelationshipComponent>(TEXT("RelationshipComponent"));
	DialogueComponent = CreateDefaultSubobject<UATRDialogueComponent>(TEXT("DialogueComponent"));
}

void AATRCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UCharacterRegistrySubsystem* Registry = GameInstance->GetSubsystem<UCharacterRegistrySubsystem>())
		{
			Registry->RegisterCharacter(this);
		}
	}
}

void AATRCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UCharacterRegistrySubsystem* Registry = GameInstance->GetSubsystem<UCharacterRegistrySubsystem>())
		{
			Registry->UnregisterCharacter(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool AATRCharacterBase::IsDialogueAvailable() const
{
	return DialogueComponent != nullptr && DialogueComponent->HasAvailableDialogue();
}
