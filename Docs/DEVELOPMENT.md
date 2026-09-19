# Development How-To

All of the below are **content changes** — no C++ recompilation required — unless noted.

## Add a character
1. Create a `UATRCharacterData` Data Asset under `Content/Data/Characters/` (Content
   Browser → Add → Miscellaneous → Data Asset → `ATRCharacterData`). Fill in `CharacterID`
   (must be unique, used everywhere else as the FName key), `DisplayName`, `Portrait`,
   `CharacterPawnClass`.
2. Create a Blueprint subclass of `AATRCharacterBase` (or `AATRPlayerCharacter` for the
   protagonist), assign the Data Asset to `CharacterData`, assign a mesh/animation blueprint.
3. Place it in a level, or spawn it from a chapter-load Blueprint.

## Add a conversation
1. Create a DataTable with row struct `FDialogueNode` (see `DialogueTypes.h`), add rows —
   each row is one beat with a `NodeID` (the row name), `SpeakerCharacterID`, `Text`, and
   an array of `Responses`.
2. Create a `UATRDialogueData` Data Asset, point `NodeTable` at it, set `EntryNodeID`,
   and optionally an `AvailabilityCondition` (story flags / relationship minimums that
   must hold for this conversation to even be offered).
3. Add the Data Asset to the NPC's `UATRDialogueComponent::AvailableConversations` array,
   in priority order (first passing condition wins — put more specific/later-story
   conversations above the generic fallback).

## Add a choice with consequences
Inside a `FDialogueResponse` row entry, fill in `Consequence`:
- `RelationshipEffects`: array of `(Axis, Delta, TargetCharacterID)` — e.g. `(Trust, +10)`.
  Leave `TargetCharacterID` empty to affect the conversation's speaker (the common case —
  the NPC's opinion of the player changes); set it explicitly only to affect a different,
  currently-registered character. See `ARCHITECTURE.md` → "Relationship ownership model".
- `StoryFlagsToSet` / `StoryFlagsToClear`: FName flags, freely invented per scene.
- `QuestEventsToFire`: FName event names matched against `UATRQuestData::StartEventName`
  and `FQuestObjective::CompletionEventName`.
- `NextNodeID`: which row to show next (empty = end conversation).

## Change relationships from outside dialogue (e.g. a gift, an action)
Call `RelationshipComponent->ModifyValue(ERelationshipAxis::Trust, 10.f)` on the target
character's `URelationshipComponent` from wherever the action happens (an item-use
Blueprint, a quest-completion hook, etc.). Dialogue is the common case, not the only one.

## Add a story flag
Story flags are just `FName`s — there is no registry to update. Pick a name
(`FoundPhotograph_003`), set it via `UStoryFlagSubsystem::SetFlag` (or a dialogue
consequence's `StoryFlagsToSet`), and reference it in any `FDialogueCondition`'s
`RequiredStoryFlags`/`ForbiddenStoryFlags`, or a `UATRQuestData`'s objective conditions.

## Add a quest
1. Create a `UATRQuestData` Data Asset. Set `QuestID`, `Title`, `Category`,
   `StartEventName` (fired to begin it), and `Objectives` (each with an `ObjectiveID`,
   `Description`, and `CompletionEventName`).
2. Register it at game/session start: call `UQuestSubsystem::RegisterQuestData(QuestData)`
   for every quest asset that should exist this session — typically from a small
   Blueprint/C++ loop over an Asset Manager query for all `UATRQuestData` assets, run once
   from `AAfterTheRainGameModeBase::BeginPlay` or a bootstrap GameInstance event. This
   loop is project-specific (depends on how quests are organized into chapters) and is the
   one piece of wiring intentionally left to the project rather than hard-coded here.
3. Fire `StartEventName`/`CompletionEventName`s from dialogue consequences, evidence
   pickups, or cinematic triggers as appropriate.

## Add a cinematic
1. Author a `ULevelSequence` in Sequencer as normal (camera cuts, character animation,
   audio tracks).
2. Place an `ACinematicTriggerActor` in the level, assign the Sequence, `CinematicID`,
   and `TriggerMode` (OnOverlap for walking into a zone, OnInteract for examining
   something first, Manual for firing from a dialogue consequence via
   `Consequence.CinematicToPlay`). The trigger registers itself with `UCinematicSubsystem`
   by `CinematicID` automatically in `BeginPlay` — a dialogue consequence naming that same
   ID will find and play it with no extra wiring.

## Add mobile touch controls
Bind a UMG on-screen joystick/buttons widget's events to the same `UInputAction`s already
consumed by `AATRPlayerCharacter` (inject an `FInputActionValue` via
`UEnhancedInputComponent`'s Action Value Injection, or simpler: call the character's
existing `Move`/`Look`/`HandleInteract` functions directly from the widget for a v1). No
gameplay code branches on platform — only the input *source* differs.
