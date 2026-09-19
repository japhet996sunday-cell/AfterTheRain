#include "Player/ATRPlayerCharacter.h"
#include "Interaction/InteractionComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Camera/ATRDialogueCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"

AATRPlayerCharacter::AATRPlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 400.f; // walk; sprint multiplies at runtime
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	DialogueCameraComponent = CreateDefaultSubobject<UATRDialogueCameraComponent>(TEXT("DialogueCameraComponent"));
}

void AATRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AATRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AATRPlayerCharacter::Move);
		if (LookAction) EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AATRPlayerCharacter::Look);
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AATRPlayerCharacter::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AATRPlayerCharacter::StopSprint);
		}
		if (CrouchAction) EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AATRPlayerCharacter::ToggleCrouch);
		if (InteractAction) EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AATRPlayerCharacter::HandleInteract);
	}
	// NOTE: On mobile, a touch-control UMG widget can call these same bound UFUNCTIONs
	// (or inject Enhanced Input action values directly) so gameplay code never branches
	// on platform.
}

void AATRPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MoveVector.Y);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MoveVector.X);
}

void AATRPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void AATRPlayerCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void AATRPlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
}

void AATRPlayerCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AATRPlayerCharacter::HandleInteract()
{
	if (InteractionComponent)
	{
		InteractionComponent->Interact();
	}
}

void AATRPlayerCharacter::SetGameplayInputEnabled(bool bEnabled)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (bEnabled)
		{
			EnableInput(PC);
		}
		else
		{
			DisableInput(PC);
		}
	}
}
