# Setup

## Requirements
- Unreal Engine **5.4** (the `.uproject` pins `EngineAssociation: 5.4`; a newer 5.x will
  prompt to convert the project — that's fine, but re-test the Enhanced Input bindings after).
- Visual Studio 2022 (Windows) with the "Game development with C++" workload, or Xcode (Mac).
- Git + Git LFS (`git lfs install`) if committing binary Content assets.

## First-time setup
1. Right-click `AfterTheRain.uproject` → **Generate Visual Studio project files**
   (or on Mac: right-click → Services → Generate Xcode project).
2. Open the generated `.sln`/`.xcodeproj`, or simply double-click `AfterTheRain.uproject`
   to let Unreal generate and open it for you.
3. Build the `AfterTheRainEditor` target (Development Editor configuration) and launch.
4. On first open, Unreal will report missing Content: this project ships with **no**
   `Content/*.uasset` files (see `README.md` — those are binary editor assets). You must
   create the handful of assets listed in `ASSET_PIPELINE.md` before the demo scene is
   playable. Budget ~20–30 minutes for a first pass following the exact specs provided.

## Creating the minimum assets to see the project run
In order, following the specs in `ASSET_PIPELINE.md` and `Content/Dialogue/DemoCafe/DemoCafeDialogue_SPEC.md`:
1. Input assets: `IMC_Default` (Input Mapping Context) + `IA_Move`, `IA_Look`, `IA_Jump`,
   `IA_Sprint`, `IA_Crouch`, `IA_Interact` (Input Actions) under `Content/Input/`.
2. A Blueprint subclass of `AATRPlayerCharacter` (e.g. `BP_PlayerCharacter`) with a
   placeholder skeletal mesh assigned and the Input assets above wired into its
   `DefaultMappingContext`/`MoveAction`/etc. properties.
3. `UATRCharacterData` assets for the protagonist and Sarah under `Content/Data/Characters/`.
   **Set `CharacterID = Sarah` exactly** on Sarah's asset — the relationship/story-flag
   fixes in this pass resolve targets by this ID at runtime (via `UCharacterRegistrySubsystem`),
   so a mismatched or empty `CharacterID` means her relationship changes silently go nowhere
   (a warning will be logged — see `DEBUGGING.md`).
4. The `DT_Sarah_FirstMeeting` DataTable and `DA_Sarah_FirstMeeting` Data Asset per the
   demo dialogue spec.
5. A Blueprint subclass of `AATRCharacterBase` for Sarah (e.g. `BP_Sarah`) with
   `DialogueComponent->AvailableConversations = [DA_Sarah_FirstMeeting]`, a placeholder
   mesh, and (for interaction to reach her) an implementation of `IInteractableInterface`
   — either add it directly to a small Blueprint wrapper, or extend `AATRCharacterBase`
   to implement it (a one-line class declaration change, left to the project since whether
   *all* characters or only interactable ones should implement it is a design call).
6. A level `Content/Maps/DemoCafe.umap` with a floor, the player start, `BP_Sarah` placed,
   and `AAfterTheRainGameModeBase` set as the level's Game Mode override (or rely on the
   project default already set in `DefaultEngine.ini`).
7. Build the Dialogue widgets and wire them to the player controller. Full exact steps
   are in a new dedicated section below — **"Dialogue UMG Setup."**
   This is the only piece of UI work required to make the café demo actually playable
   end-to-end; everything it calls into is implemented in C++.
8. Play in Editor. Approach Sarah, interact, pick a response, confirm the relationship
   value and story flag change (visible via `ATR_DumpRelationships` / `ATR_DumpStoryFlags`
   — see `DEBUGGING.md`), read Sarah's reply, tap Continue, read the goodbye line, tap
   Continue again to confirm the conversation actually ends and player control returns.

## Known editor-only gaps
See `ASSET_PIPELINE.md` for the full list — none of them are C++ work; all are content
authored in the Unreal Editor.

## Testing the corrected flow
See `TEST_CHECKLIST.md` for the step-by-step verification checklist covering player
movement, interaction, dialogue (including the corrected ending and relationship
targeting), save/load, and camera transitions.

## Dialogue UMG Setup

This section creates the two widgets described in exact detail in
`Content/UI/Dialogue/DialogueUMG_SPEC.md` — read that file alongside this one; this is
the click-by-click version, that is the field-by-field reference.

### Step 1 — Create the response button widget first (it's the dependency)
1. In the Content Browser, navigate to `Content/UI/` (create the `UI` and `Dialogue`
   folders if they don't exist yet: right-click → New Folder).
2. Right-click in `Content/UI/Dialogue/` → **User Interface → Widget Blueprint**. Name it
   `WBP_DialogueResponseButton`.
3. Double-click to open it. In the **Palette** panel (left side), drag a **Button** onto
   the canvas; it becomes the root. Rename it `ResponseButton` (right-click the widget in
   the **Hierarchy** panel → Rename, or click it and edit the name field at the top of
   **Details**).
4. Drag a **Text** widget into `ResponseButton` (drop it directly onto the button in the
   Hierarchy or canvas so it becomes its child). Rename it `ResponseButtonText`.
5. In the **Details** panel with nothing selected (click empty canvas space, or use the
   "My Blueprint" panel's Variables section), click the **+** next to **Variables** twice
   to add two variables:
   - Name `ResponseIndex`, type **Integer**. In its Details panel, check **Instance
     Editable** OFF, and check the small "eye" toggle that appears — this is **Expose on
     Spawn**; enable it.
   - Name `ResponseText`, type **Text**. Same: leave Instance Editable off, enable
     **Expose on Spawn**.
6. Switch to the **Graph** tab (top-right of the widget editor). You'll see an
   **Event Construct** node already present (if not, right-click empty graph space and
   search for it).
7. From **Event Construct**, drag off its output pin and search for **Set Text (In
   Target is Text Block)** — actually simpler: drag from `ResponseButtonText` in the
   **My Blueprint**/Hierarchy panel is not needed; instead: right-click empty graph →
   search "Set Text" filtered to Text Block target, or drag a wire from
   `ResponseButtonText` variable (click it in Hierarchy, drag into graph, choose "Get") →
   drag from its pin → search **Set Text** → connect its `Text` input to the
   `ResponseText` variable (drag `ResponseText` from My Blueprint into the graph, choose
   "Get", wire it in). Connect **Event Construct**'s exec pin into this **Set Text** node.
8. Still from **Event Construct** (chain off the same exec line, after Set Text): search
   **Get Game Instance** → drag off its output → search **Get Subsystem**, and in the
   node's class dropdown pick **Dialogue Subsystem**. Right-click the output pin →
   **Promote to Variable**; name the new variable `CachedSubsystem`.
9. In the Hierarchy panel, click `ResponseButton`. In its **Details** panel, scroll to the
   **Events** section and click the **+** next to **On Clicked** — this drops an
   **OnClicked (ResponseButton)** event node into the graph automatically.
10. From that event's exec pin, drag out and search for `CachedSubsystem` → drag from
    it → type **Select Response** → wire the `ResponseIndex` variable (Get) into its
    `Response Index` input parameter.
11. Compile (toolbar button, top-left) and Save.

### Step 2 — Create the main dialogue widget
1. In `Content/UI/Dialogue/`, right-click → **User Interface → Widget Blueprint**. Name it
   `WBP_Dialogue`.
2. Open it. In the Hierarchy, you start with a **Canvas Panel** — leave it as root.
3. Drag a **Vertical Box** into the Canvas Panel. Rename it `DialoguePanel`. With it
   selected, in **Details → Slot (Canvas Panel Slot) → Anchors**, click the preset that
   anchors to the bottom, stretched horizontally (bottom row, middle preset in the anchor
   picker). Set **Offsets**: Left = 0, Right = 0, Bottom = 0, Top = -320 (this makes the
   panel a fixed 320px band along the bottom — adjust to taste, roughly 25–30% of a
   1080px-tall viewport).
4. Drag a **Text** into `DialoguePanel`. Rename it `SpeakerNameText`. Give it a larger,
   bold font in Details → Appearance → Font.
5. Drag another **Text** into `DialoguePanel`, below the first. Rename it `DialogueText`.
   Enable **Auto Wrap Text** in its Details.
6. Drag a **Vertical Box** into `DialoguePanel`, below `DialogueText`. Rename it
   `ResponseContainer`. Leave it empty — buttons are added at runtime.
7. Drag a **Button** into `DialoguePanel`, below `ResponseContainer`. Rename it
   `ContinueButton`. Drag a **Text** inside it reading "Continue" (rename to
   `ContinueButtonLabel`).
8. (Optional, for §11 of the spec) Drag a small **Vertical Box** onto the Canvas Panel
   directly (not inside `DialoguePanel`), anchor it top-left, rename it `DebugPanel`, add
   one **Text** child named `DebugText`. Leave its own Visibility bound to a
   `bShowDebugPanel` boolean variable you'll add in the next step (Details → Behavior →
   Visibility → bind).
9. Add variables (My Blueprint panel → Variables → **+**):
   - `CachedDialogueSubsystem` — type: Object Reference → search "Dialogue Subsystem".
     Instance Editable OFF, Expose on Spawn OFF.
   - `ResponseButtonClass` — type: Class Reference → search "WBP Dialogue Response
     Button" (your Step 1 widget). Instance Editable **ON**. Expose on Spawn OFF. After
     compiling once, go to the **Class Defaults** tab and set this variable's default
     value to `WBP_DialogueResponseButton`.
   - `bShowDebugPanel` — type: Boolean. Instance Editable ON. Default False.
10. Follow `DialogueUMG_SPEC.md` §6 exactly for the Graph: bind `Event Construct` to fetch
    and cache the subsystem, bind `OnDialogueNodeShown`/`OnDialogueEnded`, and build the
    **Custom Event** `RefreshDialogueDisplay` (right-click graph → **Add Custom Event**,
    name it exactly `RefreshDialogueDisplay`) and `HandleDialogueEnded` following the
    node-by-node breakdown there. This is the part most worth reading the spec file for —
    it's the actual logic, not click-path instructions.
11. Compile and Save.

### Step 3 — Wire it into the Player Controller
1. In `Content/Blueprints/` (create if needed), right-click → **Blueprint Class** →
   search and pick **ATRPlayerController** as the parent. Name it `BP_ATRPlayerController`.
2. Open it. In **Class Defaults**, find the **UI** category and set **Dialogue Widget
   Class** to `WBP_Dialogue`.
3. Open **Project Settings → Maps & Modes** (or your `AAfterTheRainGameModeBase` Blueprint
   subclass's defaults) and set **Player Controller Class** to `BP_ATRPlayerController`.
   (If you haven't made a GameMode Blueprint subclass yet, you can instead set
   `PlayerControllerClass` directly in `AAfterTheRainGameModeBase`'s C++ constructor to
   your new Blueprint controller by making a small Blueprint subclass of the GameMode too
   — either approach works; the important part is that whichever PlayerController class is
   actually used has `DialogueWidgetClass` set.)
4. Play in Editor and follow `TEST_CHECKLIST.md`'s new UI section.

No further wiring is needed — `AATRPlayerController` (C++, already in this pass) handles
creating, showing, and hiding `WBP_Dialogue` automatically whenever
`UDialogueSubsystem::StartConversation`/`EndConversation` run.
