#include "Core/StoryFlagSubsystem.h"

void UStoryFlagSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Flags.Reset();
}

void UStoryFlagSubsystem::SetFlag(FName FlagName, bool bValue)
{
	const bool* Existing = Flags.Find(FlagName);
	const bool bChanged = (Existing == nullptr) || (*Existing != bValue);

	Flags.Add(FlagName, bValue);

	if (bChanged)
	{
		OnStoryFlagChanged.Broadcast(FlagName, bValue);
	}
}

bool UStoryFlagSubsystem::HasFlag(FName FlagName) const
{
	const bool* Existing = Flags.Find(FlagName);
	return Existing != nullptr && *Existing;
}

bool UStoryFlagSubsystem::HasAllFlags(const TArray<FName>& FlagNames) const
{
	for (const FName& FlagName : FlagNames)
	{
		if (!HasFlag(FlagName))
		{
			return false;
		}
	}
	return true;
}

void UStoryFlagSubsystem::ClearFlag(FName FlagName)
{
	if (Flags.Contains(FlagName))
	{
		Flags.Remove(FlagName);
		OnStoryFlagChanged.Broadcast(FlagName, false);
	}
}

void UStoryFlagSubsystem::RestoreFlags(const TMap<FName, bool>& InFlags)
{
	Flags = InFlags;
}
