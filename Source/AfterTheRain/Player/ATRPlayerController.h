#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ATRPlayerController.generated.h"

class AATRCharacterBase;
class UUserWidget;

/**
 * Owns nothing story-specific. Its role is the seam between gameplay and cinematics/UI:
 * cinematic and dialogue systems call SetCinematicModeActive() rather than reaching into
 * the pawn directly, so swapping how "cinematic mode" is presented (e.g. adding letterbox
 * bars) never requires touching DialogueSubsystem or CinematicTriggerActor.
 *
 * It is also the single, existing owner of the Dialogue widget's lifecycle: it binds to
 * UDialogueSubsystem's OnDialogueStarted/OnDialogueEnded once in BeginPlay and creates/
 * shows/hides one persistent widget instance accordingly. This is intentionally NOT a new
 * "UI manager" class — it's the smallest addition to an already-existing, already-central
 * actor, so dialogue UI never needs multiple unrelated Blueprints independently deciding
 * when to create or destroy it (see SETUP.md -> "Dialogue UMG Setup").
 */
UCLASS()
class AFTERTHERAIN_API AATRPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Disables pawn input and shows the UI mouse cursor (or a dialogue-safe touch layer) while true. */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetCinematicModeActive(bool bActive);

	/**
	 * Assign a Widget Blueprint deriving from UUserWidget (e.g. WBP_Dialogue) here — in a
	 * Blueprint subclass of this controller, or via a project default. Left null-safe: if
	 * unset, dialogue still functions correctly at the C++/data level, there's simply no
	 * visible UI (useful for automated/headless testing of the dialogue system itself).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> DialogueWidgetClass;

	/** Created lazily on the first dialogue and reused for every subsequent one this session. */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> DialogueWidgetInstance;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDialogueStarted(AATRCharacterBase* Speaker, AATRCharacterBase* Listener);

	UFUNCTION()
	void HandleDialogueEnded();
};
