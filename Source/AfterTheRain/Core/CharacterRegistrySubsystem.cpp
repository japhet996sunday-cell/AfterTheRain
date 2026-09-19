#include "Core/CharacterRegistrySubsystem.h"
#include "Characters/ATRCharacterBase.h"
#include "AfterTheRain.h"

void UCharacterRegistrySubsystem::RegisterCharacter(AATRCharacterBase* Character)
{
	if (!Character)
	{
		return;
	}

	const FName CharacterID = Character->GetCharacterID();
	if (CharacterID.IsNone())
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("RegisterCharacter: '%s' has no CharacterData/CharacterID set — it cannot be targeted by relationship effects, save/load, or lookup by ID."), *Character->GetName());
		return;
	}

	if (const TWeakObjectPtr<AATRCharacterBase>* Existing = RegisteredCharacters.Find(CharacterID))
	{
		if (Existing->IsValid() && Existing->Get() != Character)
		{
			UE_LOG(LogAfterTheRain, Warning, TEXT("RegisterCharacter: duplicate CharacterID '%s' — the newer instance will take precedence."), *CharacterID.ToString());
		}
	}

	RegisteredCharacters.Add(CharacterID, Character);
}

void UCharacterRegistrySubsystem::UnregisterCharacter(AATRCharacterBase* Character)
{
	if (!Character)
	{
		return;
	}

	const FName CharacterID = Character->GetCharacterID();
	if (CharacterID.IsNone())
	{
		return;
	}

	// Only remove if the registered entry is still this exact actor (avoids a late
	// EndPlay from a replaced/old instance clobbering a newer registration).
	if (const TWeakObjectPtr<AATRCharacterBase>* Existing = RegisteredCharacters.Find(CharacterID))
	{
		if (Existing->Get() == Character)
		{
			RegisteredCharacters.Remove(CharacterID);
		}
	}
}

AATRCharacterBase* UCharacterRegistrySubsystem::FindCharacter(FName CharacterID) const
{
	if (const TWeakObjectPtr<AATRCharacterBase>* Found = RegisteredCharacters.Find(CharacterID))
	{
		return Found->Get();
	}
	return nullptr;
}

TArray<FName> UCharacterRegistrySubsystem::GetAllRegisteredCharacterIDs() const
{
	TArray<FName> IDs;
	RegisteredCharacters.GenerateKeyArray(IDs);
	return IDs;
}
