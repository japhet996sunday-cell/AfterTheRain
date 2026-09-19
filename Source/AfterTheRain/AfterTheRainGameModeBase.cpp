#include "AfterTheRainGameModeBase.h"
#include "Player/ATRPlayerCharacter.h"
#include "Player/ATRPlayerController.h"

AAfterTheRainGameModeBase::AAfterTheRainGameModeBase()
{
	DefaultPawnClass = AATRPlayerCharacter::StaticClass();
	PlayerControllerClass = AATRPlayerController::StaticClass();
}

void AAfterTheRainGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}
