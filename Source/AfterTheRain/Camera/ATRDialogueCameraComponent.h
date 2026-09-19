#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ATRDialogueCameraComponent.generated.h"

class UCameraComponent;

UENUM(BlueprintType)
enum class EDialogueShotType : uint8
{
	Wide,
	OverTheShoulder,
	CloseUp,
	Tracking
};

/**
 * Lightweight cinematic-camera helper for dialogue scenes: blends to a designer-assigned
 * shot camera for the duration of a conversation, then blends back to the player's normal
 * gameplay camera. Full scripted cinematics (multi-shot sequences) should use Sequencer
 * via ACinematicTriggerActor instead — this component is specifically for the common "two
 * people talking" case so it doesn't require authoring a Sequence for every conversation.
 *
 * Attach this to the player pawn (see AATRPlayerCharacter). ShotCameraActors is populated
 * per-level/per-scene in the editor (a designer places CameraActors near the conversation
 * location and assigns them here, or on a scene-specific Blueprint that sets this map at
 * BeginPlay) — this component never assumes those actors already exist.
 */
UCLASS(ClassGroup = (AfterTheRain), meta = (BlueprintSpawnableComponent))
class AFTERTHERAIN_API UATRDialogueCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendToShot(EDialogueShotType ShotType, float BlendTime = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BlendBackToGameplay(float BlendTime = 0.5f);

	/** Assign in a level/scene Blueprint before or as a conversation starts. Empty by
	 *  default: BlendToShot logs a clear warning and no-ops for any shot type with no
	 *  camera actor assigned, rather than pretending a shot happened. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TMap<EDialogueShotType, TSoftObjectPtr<AActor>> ShotCameraActors;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Config")
	float DefaultBlendTime = 0.5f;
};
