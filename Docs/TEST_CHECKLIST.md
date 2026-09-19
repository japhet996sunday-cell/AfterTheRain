# Café Vertical Slice — Test Checklist

Manual checklist for verifying the corrected foundation once the editor assets in
`ASSET_PIPELINE.md`/`SETUP.md` exist. None of this has been runtime-tested by the AI that
authored it (no Unreal Engine instance was available) — this is the checklist for a human
(or a future AI session with engine access) to actually run.

## Player
- [ ] Player spawns at the level's PlayerStart.
- [ ] Player can move (WASD/stick) and look (mouse/stick).
- [ ] Player can approach Sarah.

## Interaction
- [ ] Sarah shows an interaction prompt when focused (`UInteractionComponent`).
- [ ] Interacting starts dialogue exactly once (no double-trigger from holding the key).
- [ ] Player movement is disabled once dialogue starts (`SetDialoguePresentationActive(true)`
      → `AATRPlayerController::SetCinematicModeActive(true)`).

## Dialogue
- [ ] Sarah's first line (`Node_Start`) appears via `OnDialogueNodeShown`.
- [ ] All three choices appear (none should be filtered — `Node_Start` has no `EntryCondition`).
- [ ] Selecting a choice applies its consequence exactly once — check the log for a single
      `Dialogue Choice: ...` line and a single `[Sarah] Trust: X -> Y` line per selection,
      not two.
- [ ] **Relationship changes apply to Sarah, not the player.** Run `ATR_DumpRelationships`
      after choosing "I just moved here..." and confirm the `Sarah | Trust: 50.0` (or
      similar) line appears — not a change under the player's own entry (the player should
      not appear in this dump at all, by design).
- [ ] Story flag `MetSarah` is set the moment the conversation starts (`ATR_DumpStoryFlags`).
- [ ] Story flag `ToldSarahTruth` or `LiedToSarah` is set after the corresponding choice.
- [ ] The correct reply node appears (`Node_TruthReply` / `Node_LieReply` / `Node_DeflectReply`)
      matching the choice made.
- [ ] That reply node's line can be read; a "Continue" control (bound to `ContinueDialogue()`)
      is shown because `IsCurrentNodeFinal()` is true.
- [ ] Tapping Continue advances to `Node_End` and its line displays.
- [ ] `Node_End`'s line can be read — it does NOT skip past automatically.
- [ ] Tapping Continue again on `Node_End` ends the conversation (`OnDialogueEnded` fires
      exactly once; check the log for exactly one `Dialogue Ended` line).
- [ ] Dialogue UI closes.
- [ ] Player movement/camera return to normal (`SetCinematicModeActive(false)` +
      `BlendBackToGameplay`).

## Dialogue UI (WBP_Dialogue / WBP_DialogueResponseButton)
- [ ] Dialogue widget appears the moment `StartConversation` succeeds (bound to
      `OnDialogueStarted` in `AATRPlayerController`, not to the first node).
- [ ] Speaker name (`SpeakerNameText`) appears and matches `Node.SpeakerCharacterID`.
- [ ] Dialogue text (`DialogueText`) appears and matches `Node.Text`.
- [ ] For `Node_Start`: three response buttons appear, `ResponseContainer` visible,
      `ContinueButton` collapsed.
- [ ] Each response button's text matches its `FDialogueResponse.ResponseText` exactly.
- [ ] Selecting a response calls `SelectResponse` exactly once (check the log for exactly
      one `Dialogue Choice:` line per click — clicking rapidly or double-tapping must not
      produce two).
- [ ] Old response buttons are gone after selecting one (`ResponseContainer.Clear
      Children` ran) — no leftover/stale buttons from the previous node visible or
      clickable underneath the new content.
- [ ] The correct next node's text appears (`Node_TruthReply`/`Node_LieReply`/
      `Node_DeflectReply` matching the choice).
- [ ] `ContinueButton` appears (and `ResponseContainer` is hidden) for that reply node,
      since it has no responses.
- [ ] The final line (`Node_End`) remains visible and readable until Continue is pressed
      — it does not auto-advance or auto-close.
- [ ] Pressing Continue on `Node_End` ends the conversation (`OnDialogueEnded` fires
      exactly once).
- [ ] The widget disappears (`RemoveFromParent` called by `AATRPlayerController`, and its
      own text/children cleared by `HandleDialogueEnded`).
- [ ] Player control (movement + camera) returns immediately after the widget disappears.
- [ ] **Keyboard:** Enter or Space activates a focused response button or the Continue
      button without a mouse click.
- [ ] **Mouse:** clicking a response button or Continue works.
- [ ] **Touch:** tapping a response button or Continue works identically to a mouse click
      (same `OnClicked` event — no separate mobile path exists to break).
- [ ] **Gamepad:** if a gamepad is connected, D-pad/stick moves focus between stacked
      response buttons, and the confirm button activates the focused one.
- [ ] Starting a second conversation later in the same session does not create a second
      `WBP_Dialogue` instance (`AATRPlayerController::DialogueWidgetInstance` is reused)
      and does not produce duplicate `OnDialogueNodeShown`/`OnDialogueEnded` log lines per
      event (which would indicate a duplicate binding).

## Save
- [ ] `ATR_TestSave MySlot` logs `SaveGame('MySlot') -> OK` with non-zero counts for
      flags/relationships matching what was just set.
- [ ] `ATR_TestLoad MySlot` logs `LoadGame('MySlot') -> OK`.
- [ ] Story flags survive load (`ATR_DumpStoryFlags` shows `MetSarah`/`ToldSarahTruth` etc.
      after loading into a fresh session).
- [ ] Sarah's relationship values survive load (`ATR_DumpRelationships`), assuming Sarah's
      actor is already present/registered in the loaded level (see the note in
      `SaveSubsystem.cpp`/`ARCHITECTURE.md` about load ordering).
- [ ] Inventory survives load (`ATR_DumpInventory`), if any items were added during testing.
- [ ] Player location survives load (character is at the saved transform, not the level's
      default PlayerStart).
- [ ] Loading the same slot twice in a row does not duplicate inventory items or double the
      relationship deltas (values match the save exactly, not save-value-plus-delta).

## Camera
- [ ] `BlendToShot` successfully cuts to an assigned shot camera if one was set up for the
      scene (`ShotCameraActors` on `UATRDialogueCameraComponent`); if none was assigned,
      confirm the expected warning is logged and the view simply stays put (not a crash,
      not a silent wrong camera).
- [ ] `BlendBackToGameplay` returns the view to the player pawn when dialogue ends.

## Quest (smoke test only — no quest content in the demo)
- [ ] `ATR_DumpQuests` runs without error even with zero registered quests.
