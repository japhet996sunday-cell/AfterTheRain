#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ATRCharacterBase.generated.h"

class UATRCharacterData;
class URelationshipComponent;
class UATRDialogueComponent;

/**
 * Common base for every named character in the world, protagonist and NPCs alike.
 * Holds identity (via UATRCharacterData), relationship state, and dialogue availability.
 * Movement/animation specifics for the player live in ATRPlayerCharacter; NPC-specific
 * behavior (schedules, AI) should subclass this rather than duplicating the shared state.
 *
 * The Mesh component is intentionally left generic (SkeletalMeshComponent from ACharacter)
 * so a MetaHuman skeletal mesh + retargeted animation blueprint can be assigned per
 * UATRCharacterData::CharacterPawnClass without any C++ changes.
 */
UCLASS(Abstract)
class AFTERTHERAIN_API AATRCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AATRCharacterBase();

	UFUNCTION(BlueprintCallable, Category = "Character")
	FName GetCharacterID() const { return CharacterData ? CharacterData->CharacterID : NAME_None; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UATRCharacterData> CharacterData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<URelationshipComponent> RelationshipComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UATRDialogueComponent> DialogueComponent;

	/** Whether this character currently has anything to say (drives interaction prompts). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character")
	bool IsDialogueAvailable() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
