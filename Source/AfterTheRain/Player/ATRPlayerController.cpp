#include "Player/ATRPlayerController.h"
#include "Player/ATRPlayerCharacter.h"
#include "Dialogue/DialogueSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AfterTheRain.h"

void AATRPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UDialogueSubsystem* Dialogue = GameInstance->GetSubsystem<UDialogueSubsystem>())
		{
			// AddUniqueDynamic-style safety: this controller's BeginPlay runs once per
			// possession, but guard anyway in case of an unusual re-possession flow —
			// IsAlreadyBound is generated for every dynamic multicast delegate.
			if (!Dialogue->OnDialogueStarted.IsAlreadyBound(this, &AATRPlayerController::HandleDialogueStarted))
			{
				Dialogue->OnDialogueStarted.AddDynamic(this, &AATRPlayerController::HandleDialogueStarted);
			}
			if (!Dialogue->OnDialogueEnded.IsAlreadyBound(this, &AATRPlayerController::HandleDialogueEnded))
			{
				Dialogue->OnDialogueEnded.AddDynamic(this, &AATRPlayerController::HandleDialogueEnded);
			}
		}
	}
}

void AATRPlayerController::HandleDialogueStarted(AATRCharacterBase* Speaker, AATRCharacterBase* Listener)
{
	if (!DialogueWidgetClass)
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("HandleDialogueStarted: no DialogueWidgetClass assigned on this controller — dialogue will run with no visible UI. Assign WBP_Dialogue in a Blueprint subclass. See SETUP.md."));
		return;
	}

	// Created once, reused for every conversation this session (add/remove from viewport
	// rather than CreateWidget every time) so repeated conversations cannot accumulate an
	// uncontrolled number of widget instances or duplicate delegate bindings.
	if (!DialogueWidgetInstance)
	{
		DialogueWidgetInstance = CreateWidget<UUserWidget>(this, DialogueWidgetClass);
	}

	if (DialogueWidgetInstance && !DialogueWidgetInstance->IsInViewport())
	{
		DialogueWidgetInstance->AddToViewport();
	}
}

void AATRPlayerController::HandleDialogueEnded()
{
	if (DialogueWidgetInstance && DialogueWidgetInstance->IsInViewport())
	{
		DialogueWidgetInstance->RemoveFromParent();
	}
}

void AATRPlayerController::SetCinematicModeActive(bool bActive)
{
	bShowMouseCursor = bActive;
	bEnableClickEvents = bActive;
	bEnableMouseOverEvents = bActive;

	if (AATRPlayerCharacter* PlayerCharacter = Cast<AATRPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->SetGameplayInputEnabled(!bActive);
	}

	SetInputMode(bActive ? FInputModeUIOnly() : FInputModeGameOnly());
}
