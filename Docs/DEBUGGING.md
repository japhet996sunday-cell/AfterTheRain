# Debugging

## Console commands (Development/Debug builds only — stripped from Shipping)
Open the console (`~` by default) and run:
- `ATR_DumpStoryFlags` — logs every set story flag and its value.
- `ATR_DumpQuests` — logs every registered quest's state and completed-objective count.
- `ATR_DumpEvidence` — logs discovered evidence IDs.
- `ATR_DumpInventory` — logs the local player's inventory.
- `ATR_DumpRelationships` — logs every registered character's relationship axis values
  (e.g. `Sarah | Trust: 50.0`). Use this to verify relationship effects are landing on the
  correct NPC, not the player.
- `ATR_TestSave <SlotName>` — saves to the given slot, logs success/failure and a summary
  of what was saved (flag/evidence/quest/relationship/item counts).
- `ATR_TestLoad <SlotName>` — loads from the given slot, logs success/failure and the same summary.

These are defined in `Debug/ATRDebugSubsystem.h/.cpp` as `UFUNCTION(Exec)` methods on a
`UGameInstanceSubsystem`, so they're available anywhere without extra setup.

## Automatic event logging
Beyond the explicit dump commands above, these fire automatically (event-based, not
per-frame) under `LogAfterTheRain`:
- `Dialogue Started (Speaker: <CharacterID>)` — once, when `StartConversation` succeeds
  (this is also the widget's cue to appear — see `DialogueUMG_SPEC.md`).
- `Dialogue Node: <NodeID> (Speaker: <CharacterID>)` — every time a node is shown.
- `Dialogue Choice: <ResponseText>` — every time a response is selected.
- `Relationship Target: <CharacterID>` — before a relationship effect is applied, so you
  can immediately see whether it resolved to the NPC you expected.
- `[<ActorName>] <Axis>: <Old> -> <New>` — every actual relationship value change.
- `Story Flag: <FlagName> = true` / `cleared` — every flag set/clear from a dialogue consequence.
- `Dialogue Ended` — exactly once per conversation, when `EndConversation()` runs.

If you ever see any of these lines fire twice for what should be a single player action
(e.g. two `Dialogue Choice:` lines for one click, or two `Dialogue Ended` lines), that's a
duplicate-event-binding bug — check for a widget or delegate bound more than once (see
`DialogueUMG_SPEC.md` §6 for why `Clear Children` in `RefreshDialogueDisplay` is what
prevents this on the response-button side, and the `IsAlreadyBound` guards in
`AATRPlayerController::BeginPlay` / `ACinematicTriggerActor::Play` for the C++ side).

## Debugging the Dialogue UI specifically
- `GetCurrentNodeID()`, `GetActiveSpeaker()`, `GetActiveListener()` on `UDialogueSubsystem`
  (all `BlueprintPure`) are the exact getters the optional in-widget debug panel uses —
  see `DialogueUMG_SPEC.md` §11. There's no separate debug-UI subsystem; it's the same
  API the widget itself consumes.
- Toggle `WBP_Dialogue`'s `bShowDebugPanel` instance variable (Class Defaults, or at
  runtime via the Blueprint debugger's Watch window) to show/hide the on-screen node/
  relationship/flag readout without touching gameplay UI.
- `ATR_DumpRelationships` (existing command, see above) is the fastest way to confirm a
  relationship change landed on the correct NPC from outside the widget entirely.

## Log categories
`LogAfterTheRain` (general), `LogATRDialogue`, `LogATRRelationship`, `LogATRQuest`,
`LogATRSave`, `LogATRCinematic` — filter the Output Log window by category when chasing
a specific system.

## Recommended next debug tooling (not yet built)
- An on-screen debug HUD (UMG or Slate) showing current dialogue node, active quest
  states, and relationship values live during Play-In-Editor — the underlying data is
  already fully queryable via the subsystems above; this is a pure UI task.
- A relationship/story-flag "cheat" panel for QA to force states without replaying content.
