#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedInteractableChanged, AActor*, NewFocus);

/**
 * Attach to the player pawn. Performs a periodic forward trace (works identically for
 * gamepad/KBM camera-forward and, later, a screen-space touch reticle on mobile) to find
 * the nearest IInteractableInterface actor, exposes it for UI prompts, and triggers it on
 * Interact(). Deliberately has no idea *what* it's interacting with — dialogue, cinematic
 * triggers, and evidence pickups all just implement IInteractableInterface.
 */
UCLASS(ClassGroup = (AfterTheRain), meta = (BlueprintSpawnableComponent))
class AFTERTHERAIN_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
	AActor* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Config")
	float TraceDistance = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Config")
	float TraceRadius = 40.f;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnFocusedInteractableChanged OnFocusedInteractableChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateFocusedInteractable();

	TWeakObjectPtr<AActor> FocusedInteractable;
};
