#include "Dialogue/ATRDialogueComponent.h"
#include "Dialogue/DialogueSubsystem.h"
#include "Characters/ATRCharacterBase.h"
#include "Kismet/GameplayStatics.h"

bool UATRDialogueComponent::HasAvailableDialogue() const
{
	// Cheap check for interaction prompts: real gating happens in GetNextAvailableConversation,
	// which needs a Listener (the player) and is called at interaction time.
	return AvailableConversations.Num() > 0;
}

UATRDialogueData* UATRDialogueComponent::GetNextAvailableConversation(AATRCharacterBase* Listener) const
{
	UDialogueSubsystem* DialogueSubsystem = UGameplayStatics::GetGameInstance(GetWorld())
		? UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDialogueSubsystem>()
		: nullptr;

	if (!DialogueSubsystem)
	{
		return nullptr;
	}

	AATRCharacterBase* Speaker = Cast<AATRCharacterBase>(GetOwner());

	for (UATRDialogueData* Conversation : AvailableConversations)
	{
		if (Conversation && DialogueSubsystem->EvaluateCondition(Conversation->AvailabilityCondition, Speaker, Listener))
		{
			return Conversation;
		}
	}
	return nullptr;
}
