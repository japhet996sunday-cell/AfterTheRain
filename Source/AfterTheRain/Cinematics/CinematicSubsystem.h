#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CinematicSubsystem.generated.h"

class ACinematicTriggerActor;

/**
 * Resolves a data-driven CinematicID (e.g. from FDialogueConsequence::CinematicToPlay) to
 * a level-placed ACinematicTriggerActor and plays it. ACinematicTriggerActor instances
 * register themselves by CinematicID in BeginPlay (mirroring UCharacterRegistrySubsystem's
 * pattern), so dialogue data never holds a direct actor reference and this subsystem never
 * hard-codes which cinematic it is.
 *
 * This is the "clean reusable hook" required by the architecture: it does not fabricate
 * playback of a Sequence that doesn't exist. If no trigger with a matching CinematicID is
 * registered in the currently loaded level, PlayCinematicByID logs a clear warning and
 * returns false rather than silently pretending to succeed.
 */
UCLASS()
class AFTERTHERAIN_API UCinematicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RegisterTrigger(ACinematicTriggerActor* Trigger);
	void UnregisterTrigger(ACinematicTriggerActor* Trigger);

	/** Returns true if a matching trigger was found and told to play. Playback itself may
	 *  still no-op if the trigger has no Sequence assigned (documented editor-asset gap). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool PlayCinematicByID(FName CinematicID);

private:
	UPROPERTY()
	TMap<FName, TWeakObjectPtr<ACinematicTriggerActor>> RegisteredTriggers;
};
