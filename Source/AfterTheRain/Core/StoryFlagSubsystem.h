#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StoryFlagSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryFlagChanged, FName, FlagName, bool, bNewValue);

/**
 * Global story-state ledger (e.g. MetSarah, LiedToSarah, FoundPhotograph_001).
 * This is the single source of truth other systems (dialogue conditions, quest gates,
 * cinematic triggers, investigation unlocks) query against. Flags are boolean by design —
 * numeric/story-integer state (relationship values, evidence counts) lives in its own
 * subsystem so this ledger stays simple, fast to check, and trivial to serialize.
 *
 * Lives on the GameInstance so it survives level travel between chapters/locations.
 */
UCLASS()
class AFTERTHERAIN_API UStoryFlagSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Sets a flag and broadcasts OnStoryFlagChanged if the value actually changed. */
	UFUNCTION(BlueprintCallable, Category = "Story|Flags")
	void SetFlag(FName FlagName, bool bValue = true);

	/** Returns false for any flag that has never been set (default-unset semantics). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story|Flags")
	bool HasFlag(FName FlagName) const;

	/** Convenience for dialogue/quest condition data: checks several flags at once (AND). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story|Flags")
	bool HasAllFlags(const TArray<FName>& FlagNames) const;

	UFUNCTION(BlueprintCallable, Category = "Story|Flags")
	void ClearFlag(FName FlagName);

	/** Full snapshot for save games. */
	TMap<FName, bool> GetAllFlags() const { return Flags; }

	/** Restores state from a loaded save game. Does not broadcast per-flag events. */
	void RestoreFlags(const TMap<FName, bool>& InFlags);

	UPROPERTY(BlueprintAssignable, Category = "Story|Flags")
	FOnStoryFlagChanged OnStoryFlagChanged;

private:
	UPROPERTY()
	TMap<FName, bool> Flags;
};
