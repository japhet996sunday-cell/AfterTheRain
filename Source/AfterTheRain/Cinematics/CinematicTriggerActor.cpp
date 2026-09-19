#include "Cinematics/CinematicTriggerActor.h"
#include "Components/BoxComponent.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/ATRPlayerController.h"
#include "Core/CharacterRegistrySubsystem.h"
#include "Cinematics/CinematicSubsystem.h"
#include "AfterTheRain.h"

ACinematicTriggerActor::ACinematicTriggerActor()
{
	PrimaryActorTick.bCanEverTick = false;

	OverlapVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapVolume"));
	RootComponent = OverlapVolume;
	OverlapVolume->SetBoxExtent(FVector(100.f));
	OverlapVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void ACinematicTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerMode == ECinematicTriggerMode::OnOverlap)
	{
		OverlapVolume->OnComponentBeginOverlap.AddDynamic(this, &ACinematicTriggerActor::HandleOverlap);
	}

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UCinematicSubsystem* CinematicSubsystem = GameInstance->GetSubsystem<UCinematicSubsystem>())
		{
			CinematicSubsystem->RegisterTrigger(this);
		}
	}
}

void ACinematicTriggerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
	{
		if (UCinematicSubsystem* CinematicSubsystem = GameInstance->GetSubsystem<UCinematicSubsystem>())
		{
			CinematicSubsystem->UnregisterTrigger(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACinematicTriggerActor::HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APawn>(OtherActor) && Cast<APawn>(OtherActor)->IsPlayerControlled())
	{
		Play();
	}
}

void ACinematicTriggerActor::Play()
{
	if (bOneShot && bHasPlayed)
	{
		return;
	}

	ULevelSequence* LoadedSequence = Sequence.LoadSynchronous();
	if (!LoadedSequence)
	{
		UE_LOG(LogAfterTheRain, Warning, TEXT("CinematicTriggerActor '%s': no Sequence assigned."), *CinematicID.ToString());
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (AATRPlayerController* ATRController = Cast<AATRPlayerController>(PC))
	{
		ATRController->SetCinematicModeActive(true);
	}

	ALevelSequenceActor* OutActor = nullptr;
	FMovieSceneSequencePlaybackSettings Settings;
	ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(this, LoadedSequence, Settings, OutActor);

	if (Player)
	{
		if (!Player->OnFinished.IsAlreadyBound(this, &ACinematicTriggerActor::HandleSequenceFinished))
		{
			Player->OnFinished.AddDynamic(this, &ACinematicTriggerActor::HandleSequenceFinished);
		}
		Player->Play();
		bHasPlayed = true;
	}

	UE_LOG(LogAfterTheRain, Log, TEXT("Playing cinematic '%s'."), *CinematicID.ToString());
}

void ACinematicTriggerActor::HandleSequenceFinished()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (AATRPlayerController* ATRController = Cast<AATRPlayerController>(PC))
	{
		ATRController->SetCinematicModeActive(false);
	}
}

FText ACinematicTriggerActor::GetInteractionPrompt_Implementation() const
{
	return FText::FromString(TEXT("Watch"));
}

bool ACinematicTriggerActor::CanInteract_Implementation(APawn* Instigator) const
{
	return TriggerMode == ECinematicTriggerMode::OnInteract && (!bOneShot || !bHasPlayed);
}

void ACinematicTriggerActor::OnInteract_Implementation(APawn* Instigator)
{
	Play();
}
