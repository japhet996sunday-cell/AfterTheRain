# Demo Content — Café Scene Dialogue Spec

**Why this is a spec file and not a `.uasset`/DataTable:** DataTables, Data Assets, and
Level Sequences are binary Unreal editor assets. They cannot be authored as text/code —
this is exactly the kind of "requires Unreal Editor setup" item called out in
`Docs/ASSET_PIPELINE.md`. What follows is the exact content to enter in-editor so the
result matches `FDialogueNode`/`FDialogueResponse` (see `Dialogue/DialogueTypes.h`)
field-for-field. Creating the DataTable takes under five minutes:

1. Content Browser → Add → Miscellaneous → Data Table → pick **FDialogueNode** as the row struct.
2. Name it `DT_Sarah_FirstMeeting`, save under `Content/Dialogue/DemoCafe/`.
3. Add the 5 rows below (row names are the `NodeID`s).
4. Create a `UATRDialogueData` asset named `DA_Sarah_FirstMeeting`, set `ConversationID = Sarah_FirstMeeting`,
   `NodeTable = DT_Sarah_FirstMeeting`, `EntryNodeID = Node_Start`, `AvailabilityCondition` = empty
   (always offered — this is the demo's only conversation).

## Relationship target (corrected)
Every `RelationshipEffect` below is written with `TargetCharacterID` left **empty**. Per
the fixed architecture (`ARCHITECTURE.md` → "Relationship ownership model"), an empty
target defaults to **the conversation's speaker** — Sarah — so "Trust +10" raises *Sarah's*
`RelationshipComponent` value (her trust in the protagonist), not the player's own. Do not
fill in `TargetCharacterID` for this demo; leave it blank so the default applies. For this
to resolve correctly at runtime, Sarah's `CharacterID` on her `UATRCharacterData` (and thus
on her placed `BP_Sarah` instance) must be exactly `Sarah` — matching what dialogue rows
reference via `SpeakerCharacterID`.

## Row: Node_Start
- SpeakerCharacterID: `Sarah`
- Text: "You're new around here, aren't you? I don't think we've met."
- EntryCondition: (none)
- Responses (3):
  1. ResponseText: "I just moved here. I'm trying to keep a low profile, honestly."
     - Consequence: RelationshipEffects = [Trust +10, TargetCharacterID: *(empty → Sarah)*], StoryFlagsToSet = [`ToldSarahTruth`], NextNodeID = `Node_TruthReply`
  2. ResponseText: "Lived here my whole life, actually." *(a lie — sets it up to matter later)*
     - Consequence: RelationshipEffects = [Trust -15, Suspicion +10, TargetCharacterID: *(empty → Sarah)*], StoryFlagsToSet = [`LiedToSarah`], NextNodeID = `Node_LieReply`
  3. ResponseText: "That's a strange question to open with."
     - Consequence: RelationshipEffects = [Suspicion +5, TargetCharacterID: *(empty → Sarah)*], NextNodeID = `Node_DeflectReply`

## Row: Node_TruthReply
- SpeakerCharacterID: `Sarah`
- Text: "Honest. I like that. Most people who come through this town aren't — that's rarer than it should be."
- Responses: (none) — AutoAdvanceNodeID: `Node_End`
- *(The player reads this line and taps Continue — see "Dialogue flow" below — which advances to Node_End.)*

## Row: Node_LieReply
- SpeakerCharacterID: `Sarah`
- Text: "...Right. Well. Enjoy your coffee." *(visibly colder — reflects the Trust drop)*
- Responses: (none) — AutoAdvanceNodeID: `Node_End`

## Row: Node_DeflectReply
- SpeakerCharacterID: `Sarah`
- Text: "Fair enough. Some things aren't anyone else's business. Yet, anyway."
- Responses: (none) — AutoAdvanceNodeID: `Node_End`

## Row: Node_End
- SpeakerCharacterID: `Sarah`
- Text: "See you around, maybe."
- Responses: (none), AutoAdvanceNodeID: (none)
- *(This is the true ending node. The corrected `UDialogueSubsystem` shows this line via
  `OnDialogueNodeShown` and stops — it does NOT auto-advance or auto-end. The player reads
  it, then dismisses it with a "Close" control bound to `ContinueDialogue()`, which sees no
  responses and no auto-advance target and calls `EndConversation()`. See
  `ARCHITECTURE.md` → "Dialogue termination and pacing".)*

## Dialogue flow (corrected)
Every node in this table — including the two mid-conversation reply nodes and `Node_End`
— has zero `Responses`, so the Dialogue UI should show a single "Continue" button for all
of them, bound to `UDialogueSubsystem::ContinueDialogue()`. Only `Node_Start` has real
choices and needs a response list bound to `SelectResponse(Index)`. A minimal Dialogue
widget can therefore branch on `IsCurrentNodeFinal()`: true → show "Continue", false →
show the response buttons from `GetAvailableResponses()`.

## What this proves about the architecture
- **Branching with consequences:** the three first responses produce three different
  relationship outcomes and two different story flags (`ToldSarahTruth` / `LiedToSarah`),
  all correctly applied to **Sarah's** relationship state, not the player's.
- **Different subsequent dialogue:** `Node_TruthReply` / `Node_LieReply` / `Node_DeflectReply`
  are genuinely different text, not the same line with a different coat of paint.
- **A later scene can branch on it for free:** any future conversation's `EntryCondition`
  can require `LiedToSarah` (forbidden) or `ToldSarahTruth` (required) with zero new code,
  and can also gate on `Sarah`'s Trust/Suspicion value directly via
  `RelationshipRequirements` (target left empty → resolves to the speaker of whichever
  conversation node checks it).
- **A free "met" flag:** starting this conversation automatically sets story flag
  `MetSarah` (derived from Sarah's `CharacterID`, not hard-coded) — see
  `UDialogueSubsystem::StartConversation`.
- **Correct exit back to gameplay:** `Node_End` displays, waits for the player, then ends
  the conversation, restores player movement/camera, and fires `OnDialogueEnded`.
