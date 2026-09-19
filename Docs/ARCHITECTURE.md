# Architecture

## Guiding principle
Story **content** (dialogue text, conditions, consequences, quest definitions, evidence
metadata, character identity) lives in data — `DataAsset`/`DataTable` instances under
`Content/`. **Systems** (C++ classes under `Source/`) are generic engines that read that
data and never contain story-specific branches. A writer should never need a C++ change
to add a character, a conversation, a choice, a quest, or a clue.

## Layer map

```
GameInstance (persists across level travel — one per play session)
├── UStoryFlagSubsystem      global boolean story state (MetSarah, LiedToSarah, ...)
├── UCharacterRegistrySubsystem  CharacterID -> currently-spawned actor lookup
├── UDialogueSubsystem       runs one conversation at a time, evaluates conditions/consequences
├── UQuestSubsystem          quest/objective state, driven by named "story events"
├── UEvidenceSubsystem       discovered-evidence set for the investigation board
├── UCinematicSubsystem      CinematicID -> level-placed ACinematicTriggerActor lookup
├── USaveSubsystem           gathers state from the subsystems above + player components
└── UATRDebugSubsystem       exec console commands, stripped from Shipping builds

Per-Actor components (attached to characters/player)
├── URelationshipComponent   that character's Trust/Romance/Friendship/Respect/Suspicion/Conflict/Affection *toward the protagonist*
├── UATRDialogueComponent    which conversations this NPC currently offers
├── UInventoryComponent      item IDs + counts (player, primarily)
├── UInteractionComponent    player-only: finds and triggers IInteractableInterface actors
└── UATRDialogueCameraComponent  player-only: blends to/from designer-assigned dialogue shot cameras

World actors
└── ACinematicTriggerActor   placeable, plays a Level Sequence, one-shot or repeatable; registers itself with UCinematicSubsystem by CinematicID
```

## Why subsystems, not singletons or a giant GameMode
`UGameInstanceSubsystem`s are Unreal's supported pattern for persistent, testable global
state: they're automatically created/destroyed with the GameInstance, survive level
travel (chapter → chapter, café → hotel), and are individually mockable/blueprint-
accessible without a global-variable smell. `AAfterTheRainGameModeBase` stays intentionally
thin — level-specific spawn logic only — because GameModes are recreated on every level
load and are the wrong place for anything that must survive a chapter transition.

## How a conversation actually runs
1. Player's `UInteractionComponent` traces forward, finds an NPC implementing
   `IInteractableInterface` (via `AATRCharacterBase` → could be extended to implement it directly,
   or via a thin wrapper Blueprint — see `DEVELOPMENT.md`).
2. On interact, code asks the NPC's `UATRDialogueComponent::GetNextAvailableConversation()`,
   which asks `UDialogueSubsystem::EvaluateCondition()` against each candidate conversation's
   `AvailabilityCondition` in priority order.
3. `UDialogueSubsystem::StartConversation()` shows the entry node; the Dialogue UMG widget
   binds to `OnDialogueNodeShown` / `OnDialogueEnded` and reads `GetAvailableResponses()`.
4. Choosing a response calls `SelectResponse(Index)`, which applies `FDialogueConsequence`
   (relationship deltas, story flags, quest events, optional cinematic) and moves to
   `NextNodeID` or ends the conversation.

## Relationship ownership model
**A character's `URelationshipComponent` holds that character's relationship *toward the
protagonist*** — e.g. Sarah's component is "how Sarah feels about the player," not the
other way around. The player's own `RelationshipComponent` (inherited like everyone else's
from `AATRCharacterBase`) is unused/meaningless and explicitly excluded from saves.

This means dialogue relationship effects and requirements default to targeting the
**conversation's speaker**, not the listener (conventionally the player):
`FRelationshipEffect`/`FRelationshipRequirement` both have an optional `TargetCharacterID`
(`NAME_None` = speaker) resolved at runtime via `UCharacterRegistrySubsystem`, which maps
`CharacterID → currently-spawned actor`. This is what makes "Trust +10" on a choice made
while talking to Sarah correctly raise *Sarah's* trust in the player, and what lets a later
node's `RelationshipRequirements` correctly read that same value back — both sides of the
condition/consequence loop resolve the target the same way, so they stay in agreement.
Explicit targets are only needed for the rare case a choice affects someone other than
whoever is currently speaking (e.g. gossip reaching a character who isn't present).

## Dialogue termination and pacing
A node is *shown* (via `OnDialogueNodeShown`) and then stays shown — the subsystem never
auto-advances past text the player hasn't acknowledged, including the very last line of a
conversation. Whenever a node has no passing responses (`IsCurrentNodeFinal() == true`),
the UI presents a single "Continue"/"Close" control bound to `ContinueDialogue()`, which
either advances to that node's `AutoAdvanceNodeID` (pure narration beats) or calls
`EndConversation()` when there's nowhere left to go (a true ending node like `Node_End`).
`EndConversation()` is itself idempotent (a no-op if already ended) and is the single place
that clears dialogue state, restores player input/camera (via
`AATRPlayerController::SetCinematicModeActive(false)` and
`UATRDialogueCameraComponent::BlendBackToGameplay()`), and broadcasts `OnDialogueEnded`.

## Consequence propagation
Dialogue consequences are the single place designers wire cross-system effects:
`FDialogueConsequence::QuestEventsToFire` calls `UQuestSubsystem::HandleStoryEvent(EventName)`,
which independently checks every registered quest's `StartEventName` and every active
quest's objective `CompletionEventName`s. Neither system needs to know about the other's
internals — they're connected only by matching `FName` event strings, so a designer can
wire a new questline entirely from data (a new `UATRQuestData` asset with the right event
names) without any code change.

## Extending without breaking existing content
- New relationship axis → add to `ERelationshipAxis` only; every consumer already reads
  through `GetValue(Axis)`, so nothing else changes.
- New condition type (e.g. "time of day") → add a field to `FDialogueCondition` and a
  check in `UDialogueSubsystem::EvaluateCondition`; existing data rows are unaffected
  (new field defaults to "no requirement").
- New platform (web) → only `Player/` (input) and a future `UI/` platform-detection layer
  should ever branch on platform; `Dialogue/`, `Quests/`, `Investigation/`, `SaveSystem/`
  have zero platform awareness by design.
