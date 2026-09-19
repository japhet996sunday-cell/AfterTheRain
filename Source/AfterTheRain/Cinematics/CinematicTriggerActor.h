#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractableInterface.h"
#include "CinematicTriggerActor.generated.h"

class ULevelSequence;
class UBoxComponent;

UENUM(BlueprintType)
enum class ECinematicTriggerMode : uint8
{
	OnOverlap,
	OnInteract,
	Manual // fired only via C++/Blueprint call, e.g. from a dialogue consequence
};

/**
 * Placeable trigger that plays a ULevelSequence (Unreal Sequencer) and hands control to/
 * from gameplay via the player controller's cinematic mode. A designer drags this into a
 * level, assigns a Sequence asset and a CinematicID, and configures the trigger mode —
 * no C++ changes needed to add a new cinematic beat.
 */
UCLASS()
class AFTERTHERAIN_API ACinematicTriggerActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ACinematicTriggerActor();

	UPROPERTY(EditAnywhere, Category = "Cinematic")
	FName CinematicID;

	UPROPERTY(EditAnywhere, Category = "Cinematic")
	TSoftObjectPtr<ULevelSequence> Sequence;

	UPROPERTY(EditAnywhere, Category = "Cinematic")
	ECinematicTriggerMode TriggerMode = ECinematicTriggerMode::OnOverlap;

	/** Consumed after first play unless false (e.g. a scene that can replay). */
	UPROPERTY(EditAnywhere, Category = "Cinematic")
	bool bOneShot = true;

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void Play();

	// IInteractableInterface
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool CanInteract_Implementation(APawn* Instigator) const override;
	virtual void OnInteract_Implementation(APawn* Instigator) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, Category = "Cinematic")
	TObjectPtr<UBoxComponent> OverlapVolume;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleSequenceFinished();

private:
	bool bHasPlayed = false;
};
