# Dialogue UMG — Exact Implementation Spec

**Why this is a spec file, not `.uasset` files:** Widget Blueprints are binary Unreal
editor assets, same as the DataTables/Data Assets in `DemoCafeDialogue_SPEC.md` — they
cannot be authored as text. Everything below is exact enough to build both widgets in
UE5.4 without guessing. Budget ~30–45 minutes for a first pass.

**Path note:** the brief suggested `Content/AfterTheRain/UI/Dialogue/`. This project's
existing convention (see `Content/Dialogue/DemoCafe/`) puts content folders directly
under `Content/`, not nested under an extra `AfterTheRain/` folder — matching the
`PROJECT_STRUCTURE` section of the original architecture brief (`Content/UI/`,
`Content/Dialogue/`, etc. as siblings). Final paths used below:
- `Content/UI/Dialogue/WBP_Dialogue.uasset`
- `Content/UI/Dialogue/WBP_DialogueResponseButton.uasset`

---

## 0. Exact existing API this widget consumes (verified against current source)

All of the following already exist in `Source/AfterTheRain/Dialogue/DialogueSubsystem.h`
and `Player/ATRPlayerController.h` as of this pass. **No further C++ changes are needed
beyond the two additions made in this same pass** (`OnDialogueStarted` delegate,
`GetCurrentNodeID`/`GetActiveSpeaker`/`GetActiveListener` getters, and the
`AATRPlayerController` widget-lifecycle wiring) — see "C++ changes made in this pass" below.

| Need | Exact API | Notes |
|---|---|---|
| Get the subsystem | `UGameInstance::GetSubsystem<UDialogueSubsystem>()` | Standard subsystem access; in Blueprint: `Get Game Instance` → `Get Subsystem (Dialogue Subsystem)`. |
| Know when to appear | `UDialogueSubsystem::OnDialogueStarted` (`FOnDialogueStarted(AATRCharacterBase* Speaker, AATRCharacterBase* Listener)`) | **Handled entirely by `AATRPlayerController`** — the widget itself does not need to bind this. See §12. |
| Know what to display | `UDialogueSubsystem::OnDialogueNodeShown` (`FOnDialogueNodeShown(const FDialogueNode& Node)`) | `Node.SpeakerCharacterID`, `Node.Text` are the two fields the widget reads directly. |
| Get current responses | `UDialogueSubsystem::GetAvailableResponses()` → `TArray<FDialogueResponse>` (BlueprintPure) | Each `FDialogueResponse.ResponseText` is what a button displays. |
| Is this a no-choice node? | `UDialogueSubsystem::IsCurrentNodeFinal()` → `bool` (BlueprintPure) | True = show Continue; False = show responses. Covers both mid-conversation narration AND the true ending node — same UI logic for both, per the corrected architecture. |
| Player picks a response | `UDialogueSubsystem::SelectResponse(int32 ResponseIndex)` | Index into the array from `GetAvailableResponses()` at the time it was displayed. |
| Player dismisses a no-choice node | `UDialogueSubsystem::ContinueDialogue()` | Ends the conversation itself if there's nowhere to advance to — the widget does not decide when to end, it just always calls this on Continue. |
| Know when it's over | `UDialogueSubsystem::OnDialogueEnded` (`FOnDialogueEnded()`, no params) | **Handled entirely by `AATRPlayerController`** for show/hide; the widget itself still binds this too, to reset its own internal state (see §13). |
| Debug info (optional panel only) | `GetCurrentNodeID()`, `GetActiveSpeaker()`, `GetActiveListener()` (all BlueprintPure) + `URelationshipComponent::GetValue(Axis)` (BlueprintPure) + `UStoryFlagSubsystem::HasFlag(Name)` (BlueprintPure) | All already exist; no new API needed for the debug panel. |

**Confirmed NOT to invent:** every function/delegate name above is copy-pasted from the
actual current header files, not guessed.

---

## 1. C++ changes made in this pass (already applied to the project)

| File | Change | Reason |
|---|---|---|
| `Dialogue/DialogueSubsystem.h/.cpp` | Added `FOnDialogueStarted` delegate, broadcast once at the top of `StartConversation` (before the entry node is shown) | The widget needs a signal to **appear** in the first place. `OnDialogueNodeShown` alone can't serve this — a widget not yet in the viewport can't react to it, and re-checking "is a widget currently visible" from inside `OnDialogueNodeShown` on every node would be exactly the kind of "several unrelated locations deciding UI state" the brief said to avoid. |
| `Dialogue/DialogueSubsystem.h` | Added `GetCurrentNodeID()`, `GetActiveSpeaker()`, `GetActiveListener()` (all `BlueprintPure`, trivial getters over already-existing private members) | Needed for the optional debug panel (§18/`DEBUGGING.md`) without exposing them as mutable state or duplicating them in Blueprint. |
| `Player/ATRPlayerController.h/.cpp` | Added `DialogueWidgetClass` (`TSubclassOf<UUserWidget>`, `EditDefaultsOnly`), `DialogueWidgetInstance`, and `BeginPlay` bindings to `OnDialogueStarted`/`OnDialogueEnded` that create-once/show/hide that instance | Something has to own "when does the widget get created and added to the viewport." `AATRPlayerController` already exists as the project's single seam between gameplay and UI/cinematic state (it already owns `SetCinematicModeActive`) — adding widget lifecycle here keeps one authority instead of introducing a new UI-manager class, per the brief's explicit instruction not to do that. |

No `DialogueSubsystem` behavior, consequence logic, relationship logic, or save logic was
touched — only additive getters/delegates.

---

## 2. WBP_Dialogue — hierarchy

```
WBP_Dialogue  (parent: User Widget)
└── CanvasPanel                         (root; auto-created)
    └── DialoguePanel                   (Vertical Box) — anchored bottom, stretched
        ├── SpeakerNameText             (Text Block)
        ├── DialogueText                (Text Block)
        ├── ResponseContainer           (Vertical Box) — holds dynamically created response buttons
        └── ContinueButton              (Button)
            └── ContinueButtonLabel     (Text Block, text = "Continue")
```

Exact names above matter — they're referenced by name throughout this spec and in
`SETUP.md`. `DialoguePanel`'s anchor: **Anchors → Bottom, Stretch horizontally**;
Position/Size roughly `Offset Top = -30%` of screen (or a fixed 280px height with anchor
at the bottom) so it occupies the lower ~25–30% of the screen, per the brief.

## 3. WBP_Dialogue — variables

| Variable | Type | Editable | Expose on Spawn | Purpose |
|---|---|---|---|---|
| `CachedDialogueSubsystem` | Object Reference → `Dialogue Subsystem` | No | No | Cached once in `Event Construct` so every subsequent call doesn't re-fetch the subsystem. **This is a cached reference, not a second copy of dialogue state** — nothing about the current node/responses is duplicated here. |
| `ResponseButtonClass` | Class Reference → `WBP_DialogueResponseButton` | **Yes** | No | Lets a level/scene Blueprint swap in a re-skinned response button later without editing this widget. |
| `bShowDebugPanel` | Boolean | Yes | No | Toggles the optional debug panel (§8). Defaults false; a developer sets this true on a debug-build instance only. |

No variable stores the current node, response list, or speaker — those are always read
live from `CachedDialogueSubsystem` via `GetAvailableResponses()` / the `Node` parameter
passed into the `OnDialogueNodeShown` event, exactly per the brief's instruction to avoid
a second copy of dialogue state.

## 4. WBP_DialogueResponseButton — hierarchy

```
WBP_DialogueResponseButton  (parent: User Widget)
└── ResponseButton           (Button) — root
    └── ResponseButtonText   (Text Block)
```

## 5. WBP_DialogueResponseButton — variables

| Variable | Type | Editable | Expose on Spawn | Purpose |
|---|---|---|---|---|
| `ResponseIndex` | Integer | No | **Yes** | Which entry in the response array this button represents. Set at creation time; never changes after. |
| `ResponseText` | Text | No | **Yes** | The text to display. Set at creation time. |

**Event Graph (WBP_DialogueResponseButton):**
```
Event Construct
    → Set Text (ResponseButtonText.Text = ResponseText)
    → Get Game Instance → Get Subsystem (Dialogue Subsystem) → store as local/instance var CachedSubsystem

ResponseButton.OnClicked
    → CachedSubsystem.SelectResponse(ResponseIndex)
```
That's the entire button. It does not touch relationships, story flags, or the next
node — it reports the index, exactly as the brief requires ("Response X was selected"),
and `UDialogueSubsystem::SelectResponse` does everything else. `OnDialogueNodeShown`/
`OnDialogueEnded` firing as a result is caught by `WBP_Dialogue`, not by this button.

---

## 6. WBP_Dialogue — Event Graph

```
Event Construct
    → Get Game Instance → Get Subsystem (Dialogue Subsystem) → Set CachedDialogueSubsystem
    → CachedDialogueSubsystem → Bind Event to On Dialogue Node Shown → RefreshDialogueDisplay
    → CachedDialogueSubsystem → Bind Event to On Dialogue Ended → HandleDialogueEnded
    → Set Visibility: Collapsed   (see note below)

    NOTE: WBP_Dialogue is added to the viewport by AATRPlayerController exactly when
    OnDialogueStarted fires (see §12/§1) — by the time Event Construct on THIS widget
    runs (the first time it's created), a conversation is already active and the first
    OnDialogueNodeShown is about to fire (or has already fired an instant before this
    widget existed, on the very first conversation of the session — see the ordering
    note in §12). Setting Collapsed here is a safety default only, immediately overridden
    by the first RefreshDialogueDisplay call.

ResponseButtonInstance click → (handled entirely inside WBP_DialogueResponseButton, see §5) → this
    triggers CachedDialogueSubsystem's own OnDialogueNodeShown or OnDialogueEnded, which
    this graph already listens for → RefreshDialogueDisplay or HandleDialogueEnded runs.

ContinueButton.OnClicked
    → CachedDialogueSubsystem.ContinueDialogue()
    → (as above: triggers OnDialogueNodeShown or OnDialogueEnded, already bound)
```

### Custom Function: `RefreshDialogueDisplay(Node: FDialogueNode)`
Bound to `OnDialogueNodeShown`; `Node` is the event's own parameter — do not re-fetch it.

```
1. ResponseContainer → Clear Children
   (destroys any previous response button widgets outright — this is what prevents
   stale buttons AND duplicate event bindings: a destroyed widget's bindings go with it,
   there is nothing left to unbind manually)

2. Set Text: SpeakerNameText.Text = Node.SpeakerCharacterID (as Text)

3. Set Text: DialogueText.Text = Node.Text

4. Local Bool: bIsFinal = CachedDialogueSubsystem.IsCurrentNodeFinal()

5. Branch on bIsFinal:
   TRUE:
     - ResponseContainer → Set Visibility: Collapsed
     - ContinueButton → Set Visibility: Visible
     - Set Keyboard Focus: ContinueButton
   FALSE:
     - ContinueButton → Set Visibility: Collapsed
     - ResponseContainer → Set Visibility: Visible
     - Get CachedDialogueSubsystem.GetAvailableResponses() → For Each Loop (with index):
         - Create Widget (Class = ResponseButtonClass, Owning Player = Get Owning Player)
         - Set its ResponseIndex = loop index, ResponseText = array element's ResponseText
           (both via the Create Widget node's Expose-on-Spawn pins, filled in at creation —
            no separate "Initialize" call needed)
         - ResponseContainer → Add Child (the new button)
     - After the loop: Set Keyboard Focus on the FIRST created response button
       (cache it in a local variable during the loop's first iteration)

6. If bShowDebugPanel: call RefreshDebugPanel (see §8) — otherwise skip.

7. Self → Set Visibility: Visible
   (belt-and-suspenders: guarantees the widget is visible whenever a node is actually
   being shown, independent of the AddToViewport call already made by the controller)
```

### Custom Function: `HandleDialogueEnded()`
Bound to `OnDialogueEnded`.
```
1. ResponseContainer → Clear Children
2. Set Text: DialogueText.Text = "" ; SpeakerNameText.Text = ""
3. Self → Set Visibility: Collapsed
```
Note: `AATRPlayerController::HandleDialogueEnded` (C++) already calls
`RemoveFromParent()` on this same widget instance — this Blueprint function additionally
clears displayed text/children so that if the SAME widget instance is reused for the next
conversation (which it will be — see §12), it doesn't briefly flash the previous
conversation's last line before the next `OnDialogueNodeShown` arrives.

---

## 7. Input Mode / Focus (already handled by existing C++ — document, don't duplicate)

`AATRPlayerController::SetCinematicModeActive(true)`, already called by
`UDialogueSubsystem::StartConversation` via `SetDialoguePresentationActive`, sets:
- `SetInputMode(FInputModeUIOnly())`
- `bShowMouseCursor = true`
- Pawn gameplay input disabled via `SetGameplayInputEnabled(false)`

**Do not set input mode from the widget.** The widget only calls `Set Keyboard Focus` on
its own buttons (§6, step 5) — that's focus *within* UI-only input mode, not a mode
change, and is exactly the minimum state change the brief asked for. When
`SetCinematicModeActive(false)` runs on conversation end, input mode reverts to
`FInputModeGameOnly()` automatically — nothing in the widget needs to restore it.

**Keyboard:** native `UButton` widgets respond to Gamepad/Keyboard "Accept" (Enter /
Gamepad Face Button Bottom) and Space when focused, by default, with no extra Blueprint
wiring — this covers the brief's "Enter/Space for Continue" requirement as long as
`Set Keyboard Focus` (§6) is called after every refresh. Number-key response shortcuts
are intentionally NOT implemented, per the brief's "do not implement unless the existing
input architecture supports them cleanly" — there is no existing numbered-input binding
to hang this off of.

**Gamepad navigation between multiple response buttons:** set each `WBP_DialogueResponseButton`'s
`ResponseButton`'s Navigation rule to `Down`/`Up` = `Auto` in its widget Details panel —
Slate's automatic focus navigation handles moving between stacked buttons in a
`VerticalBox` with no Blueprint logic required.

**Mouse/Touch:** native `Button.OnClicked` already fires identically for a mouse click or
a touch tap — no separate mobile path is needed, satisfying "do not create a separate
mobile dialogue system."

---

## 8. Mobile sizing

- `DialoguePanel`: minimum height ~220px at 1080p-equivalent scale (roughly 20% of a
  1080-tall viewport), scaling via a Scale Box or DPI curve if the project sets one up
  later (not required for this pass).
- `ResponseButton`/`ContinueButton`: minimum touch target 88x44px (a commonly-cited
  comfortable minimum), with at least 8px vertical padding between stacked response
  buttons (`ResponseContainer`'s Vertical Box slot padding).
- `DialogueText`/`SpeakerNameText`: minimum 18pt at 1080p-equivalent, white or near-white
  on the panel's dark translucent background for contrast.
- No hover-only affordances: buttons must be fully operable via a single tap, which native
  `UButton` already satisfies (its "hovered" visual state is cosmetic only, never gating
  the click).

## 9. Visual style (prototype-level, per the brief — no final art)

- `DialoguePanel` background: a plain `Image` behind the Vertical Box, solid dark color
  (e.g. `(0,0,0,0.75)` RGBA) — a translucent dark panel, no texture asset required (Unreal's
  default white "Border"/`SImage` brush tinted via Color and Opacity is sufficient).
- Text: white/near-white (`(0.95, 0.95, 0.95, 1.0)`), `SpeakerNameText` bold, ~20% larger
  than `DialogueText`.
- Buttons: flat-colored rectangles (default `UButton` style, tinted), no borrowed/branded
  assets — satisfies "no copyrighted UI assets."

---

## 10. Widget creation / session behavior

Fully covered by the C++ addition in §1: `AATRPlayerController` creates
`DialogueWidgetInstance` **once**, lazily, on the first `OnDialogueStarted`, and reuses it
for every subsequent conversation via `AddToViewport()`/`RemoveFromParent()` rather than
`CreateWidget` each time. Repeated conversations therefore cannot accumulate multiple
widget instances or duplicate delegate bindings — `WBP_Dialogue`'s own `Event Construct`
(where its subsystem bindings happen) only runs once, the first time it's created.

**Ordering note:** because `OnDialogueStarted` broadcasts before `ShowNode` runs (see the
`StartConversation` source), and widget creation happens synchronously inside that
delegate's handler, `WBP_Dialogue` is guaranteed to exist and have run `Event Construct`
(and therefore be bound to `OnDialogueNodeShown`) before the entry node's
`OnDialogueNodeShown` broadcasts moments later in the same call stack. No race condition.

---

## 11. Optional Debug Panel

Add a `DebugPanel` (Vertical Box, top-left corner, small text, only visible when
`bShowDebugPanel == true`) containing a single `DebugText` (Text Block) rebuilt by a
`RefreshDebugPanel` function (called from step 6 of `RefreshDialogueDisplay`):

```
RefreshDebugPanel:
  Speaker = CachedDialogueSubsystem.GetActiveSpeaker()
  Format a multi-line string:
    "Node: " + CachedDialogueSubsystem.GetCurrentNodeID()
    "Speaker: " + (Speaker ? Speaker.GetCharacterID() : "none")
    "Trust: " + Speaker.RelationshipComponent.GetValue(Trust)          [if Speaker valid]
    "Romance: " + Speaker.RelationshipComponent.GetValue(Romance)
    "Friendship/Respect/Suspicion/Conflict/Affection" — same pattern
  Set DebugText.Text = the formatted string
```
No new debug subsystem or console command is needed for this — it's reading the same
`BlueprintPure` getters the rest of the widget already uses, per the brief's instruction
to use the existing debug subsystem rather than build a second one. For story flags, add
one line per flag of interest, e.g. `"MetSarah: " + StoryFlagSubsystem.HasFlag('MetSarah')`
via `Get Game Instance → Get Subsystem (Story Flag Subsystem) → Has Flag`.
