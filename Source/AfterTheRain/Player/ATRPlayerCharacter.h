#pragma once

#include "CoreMinimal.h"
#include "Characters/ATRCharacterBase.h"
#include "ATRPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInteractionComponent;
class UInventoryComponent;
class UATRDialogueCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * The protagonist. Movement uses Enhanced Input so the same action bindings can be
 * re-mapped to a virtual joystick + touch buttons on mobile (via a separate mobile
 * UMG widget calling the same UInputAction triggers) without duplicating gameplay code.
 *
 * CharacterData->CharacterPawnClass on the protagonist's UATRCharacterData is where a
 * MetaHuman-derived Blueprint subclass of this class would eventually be assigned;
 * this base class never hard-codes a specific skeletal mesh.
 */
UCLASS()
class AFTERTHERAIN_API AATRPlayerCharacter : public AATRCharacterBase
{
	GENERATED_BODY()

public:
	AATRPlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UATRDialogueCameraComponent> DialogueCameraComponent;

	/** Called by dialogue/cinematic systems to hand control to/from gameplay. */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetGameplayInputEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Enhanced Input assets — assign in the Blueprint subclass or a per-platform DataAsset.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void ToggleCrouch();
	void HandleInteract();
};
