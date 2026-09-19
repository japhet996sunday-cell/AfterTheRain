#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class AFTERTHERAIN_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by anything the player can interact with: NPCs (start dialogue), evidence
 * pickups, doors, phones, etc. Keeping this as a thin interface (rather than requiring
 * everything to derive from one base Actor class) lets furniture, NPCs, and future phone/
 * clue objects share one interaction path through UInteractionComponent.
 */
class AFTERTHERAIN_API IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionPrompt() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(APawn* Instigator) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteract(APawn* Instigator);
};
