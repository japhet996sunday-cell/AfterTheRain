#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CharacterRegistrySubsystem.generated.h"

class AATRCharacterBase;

/**
 * Runtime lookup of "CharacterID -> spawned actor instance" for whichever characters are
 * currently loaded in the world. This is the piece that lets other systems resolve a
 * data-driven FName (e.g. a dialogue consequence's relationship target, or a save file's
 * relationship entry) to an actual actor WITHOUT hard-coding which character it is.
 *
 * Characters register themselves in AATRCharacterBase::BeginPlay and unregister in
 * EndPlay, so the registry always reflects "who currently exists in the loaded level(s)".
 * It intentionally does not persist across level travel by itself — characters entering
 * the new level re-register in their own BeginPlay, same as any other actor-lifetime state.
 */
UCLASS()
class AFTERTHERAIN_API UCharacterRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RegisterCharacter(AATRCharacterBase* Character);
	void UnregisterCharacter(AATRCharacterBase* Character);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Characters")
	AATRCharacterBase* FindCharacter(FName CharacterID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Characters")
	TArray<FName> GetAllRegisteredCharacterIDs() const;

private:
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<AATRCharacterBase>> RegisteredCharacters;
};
