#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableInterface.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz is plenty for interaction focus checks
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocusedInteractable();
}

void UInteractionComponent::UpdateFocusedInteractable()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	const UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>();
	const FVector TraceStart = Camera ? Camera->GetComponentLocation() : OwnerPawn->GetActorLocation();
	const FVector TraceDirection = Camera ? Camera->GetForwardVector() : OwnerPawn->GetActorForwardVector();
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius), Params);

	AActor* NewFocus = nullptr;
	if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<UInteractableInterface>())
	{
		NewFocus = Hit.GetActor();
	}

	if (FocusedInteractable.Get() != NewFocus)
	{
		FocusedInteractable = NewFocus;
		OnFocusedInteractableChanged.Broadcast(NewFocus);
	}
}

void UInteractionComponent::Interact()
{
	AActor* Target = FocusedInteractable.Get();
	if (!Target || !Target->Implements<UInteractableInterface>())
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (IInteractableInterface::Execute_CanInteract(Target, OwnerPawn))
	{
		IInteractableInterface::Execute_OnInteract(Target, OwnerPawn);
	}
}
