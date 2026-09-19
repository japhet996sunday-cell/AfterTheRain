#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AfterTheRainGameModeBase.generated.h"

/**
 * Base game mode for After the Rain.
 * Kept intentionally thin: persistent, cross-chapter state lives in GameInstanceSubsystems
 * (StoryFlagSubsystem, DialogueSubsystem, QuestSubsystem, etc.) so it survives level travel.
 * Per-level setup (e.g. spawning chapter-specific actors) can be added here or in a
 * level-specific Blueprint subclass without touching the subsystems.
 */
UCLASS()
class AFTERTHERAIN_API AAfterTheRainGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAfterTheRainGameModeBase();

protected:
	virtual void BeginPlay() override;
};
