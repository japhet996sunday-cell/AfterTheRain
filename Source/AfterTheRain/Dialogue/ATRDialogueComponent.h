#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/DialogueTypes.h"
#include "ATRDialogueComponent.generated.h"

class AATRCharacterBase;

/**
 * Attached to an NPC (or the player, for symmetry). Holds the list of conversations this
 * character *could* offer and picks the first one whose AvailabilityCondition currently
 * passes — this is how the same NPC offers different conversations over the course of
 * the story (e.g. first meeting vs. post-secret-revealed) without extra branching logic
 * in the interaction code: designers just add DialogueData entries in priority order.
 */
UCLASS(ClassGroup = (AfterTheRain), meta = (BlueprintSpawnableComponent))
class AFTERTHERAIN_API UATRDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Highest-priority conversations first; the first one whose condition passes is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<TObjectPtr<UATRDialogueData>> AvailableConversations;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialogue")
	bool HasAvailableDialogue() const;

	/** Returns the highest-priority conversation currently available, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	UATRDialogueData* GetNextAvailableConversation(AATRCharacterBase* Listener) const;
};
