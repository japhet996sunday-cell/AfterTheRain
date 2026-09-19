#include "Cinematics/CinematicSubsystem.h"
#include "Cinematics/CinematicTriggerActor.h"
#include "AfterTheRain.h"

void UCinematicSubsystem::RegisterTrigger(ACinematicTriggerActor* Trigger)
{
	if (Trigger && !Trigger->CinematicID.IsNone())
	{
		RegisteredTriggers.Add(Trigger->CinematicID, Trigger);
	}
}

void UCinematicSubsystem::UnregisterTrigger(ACinematicTriggerActor* Trigger)
{
	if (!Trigger || Trigger->CinematicID.IsNone())
	{
		return;
	}

	if (const TWeakObjectPtr<ACinematicTriggerActor>* Existing = RegisteredTriggers.Find(Trigger->CinematicID))
	{
		if (Existing->Get() == Trigger)
		{
			RegisteredTriggers.Remove(Trigger->CinematicID);
		}
	}
}

bool UCinematicSubsystem::PlayCinematicByID(FName CinematicID)
{
	if (const TWeakObjectPtr<ACinematicTriggerActor>* Found = RegisteredTriggers.Find(CinematicID))
	{
		if (ACinematicTriggerActor* Trigger = Found->Get())
		{
			Trigger->Play();
			return true;
		}
	}

	UE_LOG(LogAfterTheRain, Warning, TEXT("PlayCinematicByID: no ACinematicTriggerActor with CinematicID '%s' is registered in the current level. Place one and assign a Level Sequence to enable this beat."), *CinematicID.ToString());
	return false;
}
