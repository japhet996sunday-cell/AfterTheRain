#include "Camera/ATRDialogueCameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "AfterTheRain.h"

void UATRDialogueCameraComponent::BlendToShot(EDialogueShotType ShotType, float BlendTime)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	const TSoftObjectPtr<AActor>* FoundSoftActor = ShotCameraActors.Find(ShotType);
	AActor* CameraActor = FoundSoftActor ? FoundSoftActor->LoadSynchronous() : nullptr;

	if (!CameraActor)
	{
		// Honest foundation, not a fake camera cut: without an assigned camera actor for
		// this shot type, the view simply stays where it is. See ASSET_PIPELINE.md.
		UE_LOG(LogAfterTheRain, Warning, TEXT("BlendToShot: no camera actor assigned for shot type %d — assign one in ShotCameraActors for this scene to enable it."), (int32)ShotType);
		return;
	}

	PC->SetViewTargetWithBlend(CameraActor, BlendTime);
}

void UATRDialogueCameraComponent::BlendBackToGameplay(float BlendTime)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		PC->SetViewTargetWithBlend(Pawn, BlendTime);
	}
}
