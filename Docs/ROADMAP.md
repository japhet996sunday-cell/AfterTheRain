# Roadmap

## Phase 0 — Foundation (initial delivery)
Core subsystems (story flags, dialogue, relationships, quests, evidence, inventory,
save/load, debug), player/interaction/cinematic scaffolding, and a fully specified café
demo scene proving branching dialogue with real consequences. C++ only — no editor assets
included (can't be, they're binary).

## Phase 0.5 — Foundation Fix Pass (this delivery) — COMPLETE
Correctness pass over Phase 0's architecture. See the chat response accompanying this zip
for the full bug list; summary of what's now fixed:
- Dialogue termination: nodes with no responses no longer auto-chain past unread text; a
  new `ContinueDialogue()`/`IsCurrentNodeFinal()` pair lets the UI show the final line and
  end the conversation only once the player dismisses it. Consequence double-application
  is now structurally prevented (response list is cleared before the consequence runs).
- Relationship ownership: `URelationshipComponent` is documented and enforced as "this
  character's relationship toward the protagonist." Dialogue relationship effects AND
  conditions now both default to targeting the conversation's **speaker**, resolved at
  runtime via the new `UCharacterRegistrySubsystem` — reusable for any future NPC by
  `CharacterID`, no hard-coded names.
- Save/load: relationships and quest state are now genuinely gathered and restored (via
  `UCharacterRegistrySubsystem` and new `UQuestSubsystem` accessors), not just present in
  the schema. All restores are wholesale replacements, so repeated loads can't duplicate data.
- Cinematic hook: `FDialogueConsequence::CinematicToPlay` now resolves through a real
  `UCinematicSubsystem` registry to a level-placed `ACinematicTriggerActor`, instead of
  only logging.
- Camera foundation: `UATRDialogueCameraComponent::BlendToShot` performs an actual
  `SetViewTargetWithBlend` against designer-assigned shot cameras (still requires those
  camera actors to be placed per scene — that's expected editor work, not a code gap).
- Input: starting/ending a conversation now actually calls
  `AATRPlayerController::SetCinematicModeActive`, which was previously never invoked by
  the dialogue system at all.
- A `Met<CharacterID>` story flag is now set automatically and generically when any
  conversation with that character starts.

## Phase 0.75 — Dialogue UMG Implementation Package (this delivery) — SPEC COMPLETE, WIDGETS NOT YET BUILT
Prepared the project so the café vertical slice can become playable with a real UI. This
pass is documentation + two small C++ additions, not new widgets (those are binary editor
assets — see `ASSET_PIPELINE.md`).
- Added `UDialogueSubsystem::OnDialogueStarted` — the missing "appear now" signal a widget
  needs (previously only "node shown" and "ended" existed).
- Added `GetCurrentNodeID()`/`GetActiveSpeaker()`/`GetActiveListener()` read-only getters
  for the optional debug panel.
- `AATRPlayerController` now owns the Dialogue widget's lifecycle (create-once, show/hide
  on `OnDialogueStarted`/`OnDialogueEnded`) — no new UI-manager class, no scattered
  Blueprint logic deciding when the widget should exist.
- Full field-by-field spec for `WBP_Dialogue` and `WBP_DialogueResponseButton` in
  `Content/UI/Dialogue/DialogueUMG_SPEC.md`, plus click-by-click editor steps in
  `SETUP.md` → "Dialogue UMG Setup."
- `TEST_CHECKLIST.md` and `DEBUGGING.md` extended with UI-specific verification steps.
- **Not done:** the actual `.uasset` Widget Blueprints. Building them per the spec is the
  single remaining blocker for a playable café demo.

## Phase 1 — Make the foundation playable (recommended immediate next step)
- Create the handful of editor assets listed in `ASSET_PIPELINE.md`'s first table,
  including `WBP_Dialogue`/`WBP_DialogueResponseButton` per `DialogueUMG_SPEC.md` — this
  is now the only remaining piece of work standing between this codebase and a playable demo.
- Build a basic Main Menu (New/Continue/Settings) and Pause widget.
- Verify the full loop using `TEST_CHECKLIST.md`: menu → gameplay → interact → dialogue →
  consequence → save → load.
- Place at least one `ACinematicTriggerActor` with a real Sequence to exercise the new
  `UCinematicSubsystem` hook end-to-end.

## Phase 2 — First real chapter
- Replace placeholder mannequins with MetaHuman (or equivalent) for protagonist + 1–2 key
  characters; wire facial animation hooks.
- Author one full chapter: multiple locations, one investigation thread (2–3 evidence
  items), one romance-relevant character, 2–3 cinematic beats via Sequencer.
- Build Inventory and Evidence-board UMG screens.
- Mobile touch-control widget; verify Android/iOS packaging end-to-end (even with
  placeholder art) to catch platform issues early rather than late.
- Wire the quest registration loop (`UQuestSubsystem::RegisterQuestData` for every quest
  asset) once chapter structure is decided — still the one piece of quest wiring left to
  the project, per `DEVELOPMENT.md`.

## Phase 3 — Systems hardening
- Music-state subsystem (Normal/Romantic/Tense/Mystery/Sad/Danger) once music assets exist.
- Settings screen wired to Scalability CVars and audio buses.
- Localization Dashboard turned on; verify no hard-coded `FString` slipped into
  player-facing text.
- Analytics subsystem attached to the story/quest/relationship event delegates, once a
  provider is chosen.
- Save versioning exercised: intentionally bump `ATR_SAVE_VERSION_CURRENT` and write a
  migration to prove the seam works before it's load-bearing.
- Save slot enumeration (`USaveSubsystem::GetAvailableSaveSlots`) for the target platform.

## Phase 4 — Content scale-up
- Multiple chapters, full relationship-outcome branching, replay-a-chapter support.
- Full art pass: environments, lighting, VO, final music.
- Performance pass with real assets: LOD, streaming, mobile scalability tiers.

## Known limitations after this fix pass
- **Not compiled or runtime-tested.** No Unreal Engine installation was available in this
  environment. Every change was verified by static review only — see the "Verification"
  section of the accompanying chat response for exactly what that covered.
- No binary editor assets are included (DataTables, Data Assets, Levels, Sequences,
  Input assets) — see `ASSET_PIPELINE.md` for the exact specs to create them.
- No UI *assets* have been built yet — the C++ side (widget lifecycle, all data the UI
  needs) is ready; `WBP_Dialogue`/`WBP_DialogueResponseButton` themselves are the critical
  path item for Phase 1, fully spec'd in `DialogueUMG_SPEC.md`.
- `USaveSubsystem::GetAvailableSaveSlots()` is still an unimplemented stub (logs and
  returns empty) — platform-specific, tracked for Phase 3.
- Quest registration-at-session-start loop is still a project-specific decision, tracked
  for Phase 2 once chapter structure exists.
