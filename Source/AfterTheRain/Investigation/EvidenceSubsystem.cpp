#include "Investigation/EvidenceSubsystem.h"
#include "Investigation/EvidenceTypes.h"
#include "Core/StoryFlagSubsystem.h"

void UEvidenceSubsystem::DiscoverEvidence(FName EvidenceID)
{
	bool bAlreadyInSet = false;
	DiscoveredIDs.Add(EvidenceID, &bAlreadyInSet);

	if (!bAlreadyInSet)
	{
		OnEvidenceDiscovered.Broadcast(EvidenceID);
	}
}
