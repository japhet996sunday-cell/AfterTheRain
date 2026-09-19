# Game Systems Reference

## Story Flags (`Core/StoryFlagSubsystem`)
Boolean global state, default-unset. `SetFlag`/`HasFlag`/`HasAllFlags`/`ClearFlag`.
Broadcasts `OnStoryFlagChanged` for UI/audio/ambient reactions (e.g. a radio playing a
different track once `DiscoveredSecret` is true).

## Relationships (`Characters/RelationshipComponent`)
Seven axes (`ERelationshipAxis`): Trust, Romance, Friendship, Respect, Suspicion,
Conflict, Affection. Clamped to a configurable `[MinValue, MaxValue]` (default -100..100).
`MeetsThreshold(Axis, Min)` is the standard way to gate content — thresholds live in
dialogue/quest *data*, never hard-coded in this component. **Ownership:** a character's
component holds that character's relationship *toward the protagonist* (Sarah's component
= Sarah's trust in the player), resolved by dialogue via `UCharacterRegistrySubsystem` —
see `ARCHITECTURE.md`.

## Character Registry (`Core/CharacterRegistrySubsystem`)
Maps `CharacterID -> currently-spawned AATRCharacterBase*`. Characters register in
`BeginPlay`/unregister in `EndPlay`. This is how dialogue consequences/conditions and the
save system resolve a data-driven `FName` to an actual actor without any hard-coded
per-character branches.

## Dialogue (`Dialogue/*`)
Data-driven conversation graphs (`FDialogueNode` rows in a `DataTable`, wrapped by a
`UATRDialogueData` asset). `UDialogueSubsystem` runs exactly one conversation at a time,
evaluates `FDialogueCondition`s against story flags / relationships / inventory / quest
state, and applies `FDialogueConsequence`s. `UATRDialogueComponent` on each NPC picks
which conversation to offer based on priority + availability conditions.

## Quests (`Quests/*`)
`UATRQuestData` assets define objectives; `UQuestSubsystem` tracks runtime state
(`EQuestState`: Unknown/Hidden/Active/Completed/Failed) and completed-objective sets,
keyed by `FName QuestID`. Everything is driven by "story events" (`HandleStoryEvent`) so
quests never need direct references to whatever triggered them.

## Investigation / Evidence (`Investigation/*`)
`UATREvidenceData` assets describe a clue's display info and story linkage
(`RelatedCharacterIDs`, `RelatedLocationID`). `UEvidenceSubsystem` tracks which
`EvidenceID`s have been discovered — feed an evidence-board UMG widget from
`GetAllDiscovered()`.

## Inventory (`Inventory/*`)
`UATRItemData` assets define items (icon, category, stackability, story relevance);
`UInventoryComponent` tracks only `(ItemID, Count)` pairs at runtime, kept intentionally
separate from Evidence since evidence needs board/relationship metadata items don't.

## Interaction (`Interaction/*`)
`IInteractableInterface` (BlueprintNativeEvent: `GetInteractionPrompt`, `CanInteract`,
`OnInteract`) is implemented by anything the player can act on. `UInteractionComponent`
(on the player) sweeps forward at 10Hz, exposes the focused interactable for UI prompts,
and calls `OnInteract` when the player presses Interact.

## Cinematics (`Cinematics/*`) & Camera (`Camera/*`)
`ACinematicTriggerActor` plays a `ULevelSequence` on overlap, interact, or manual call,
and hands input control to/from `AATRPlayerController::SetCinematicModeActive`. It
registers itself by `CinematicID` with `UCinematicSubsystem`, which is how a dialogue
consequence's `CinematicToPlay` (a plain FName) reaches an actual placed trigger without
holding a direct actor reference. `UATRDialogueCameraComponent` (attached to the player)
is the lighter-weight path for ordinary two-person dialogue shots that don't warrant
authoring a full Sequence: `BlendToShot` cuts to a designer-assigned camera actor from its
`ShotCameraActors` map, logging a clear warning (not a fake cut) if none is assigned for
that scene yet.

## Save/Load (`SaveSystem/*`)
`UATRSaveGame` is a flat, versioned (`ATR_SAVE_VERSION_CURRENT`) snapshot with no UObject
references. `USaveSubsystem` gathers state from the other subsystems and the player's
components on save, and restores it on load. `MigrateIfNeeded` is the seam for schema
migrations across future save-version bumps.

## Debug (`Debug/ATRDebugSubsystem`)
Console `Exec` functions (`ATR.ATR_DumpStoryFlags`, `ATR_DumpEvidence`, `ATR_DumpInventory`,
`ATR_TestSave`, `ATR_TestLoad`) compiled out via `#if !UE_BUILD_SHIPPING`. Per-system log
categories (`LogATRDialogue`, `LogATRRelationship`, `LogATRQuest`, `LogATRSave`,
`LogATRCinematic`) are declared for filtering Output Log verbosity per system.

## Data structure summary
| Struct/Class | Purpose |
|---|---|
| `UATRCharacterData` | Character identity (name, portrait, pawn class, traits) |
| `FDialogueNode` / `UATRDialogueData` | One conversation graph |
| `FDialogueCondition` / `FDialogueConsequence` | Gating and effects, reused by nodes and responses |
| `FRelationshipRequirement` / `FRelationshipEffect` | Single-axis threshold or delta, with an optional `TargetCharacterID` (default: speaker) |
| `UATRQuestData` / `FQuestObjective` | Quest definition |
| `UATRItemData` / `FInventoryEntry` | Item definition / runtime possession |
| `UATREvidenceData` | Clue definition |
| `UATRSaveGame` | Full save snapshot |
