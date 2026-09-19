# Asset Pipeline — What's Placeholder and What You Must Supply

Nothing in this codebase fakes photorealism with primitives or text. Instead, every
system exposes a clean slot for the real asset. This is the authoritative list.

## Must be created in Unreal Editor before the project runs (no art skill required)
| Asset | Path | Notes |
|---|---|---|
| `IMC_Default` + `IA_*` Input assets | `Content/Input/` | Enhanced Input; ~10 min, see `SETUP.md` |
| `UATRCharacterData` (protagonist, Sarah) | `Content/Data/Characters/` | Text fields only |
| `DT_Sarah_FirstMeeting` + `DA_Sarah_FirstMeeting` | `Content/Dialogue/DemoCafe/` | Exact spec in `DemoCafeDialogue_SPEC.md` |
| `WBP_DialogueResponseButton` | `Content/UI/Dialogue/` | Exact spec in `DialogueUMG_SPEC.md`; build this one first (it's a dependency of the next) |
| `WBP_Dialogue` | `Content/UI/Dialogue/` | Exact spec in `DialogueUMG_SPEC.md`; the main dialogue panel |
| `BP_ATRPlayerController` | `Content/Blueprints/` | Blueprint subclass of `AATRPlayerController` with `DialogueWidgetClass = WBP_Dialogue` set |
| `BP_PlayerCharacter`, `BP_Sarah` | `Content/Blueprints/` | Blueprint subclasses wiring the above |
| `DemoCafe` level | `Content/Maps/` | Floor + player start + `BP_Sarah` placed; Game Mode's PlayerControllerClass set (directly or via a GameMode Blueprint) to `BP_ATRPlayerController` |

## Placeholder now, real production asset later
| System | Current placeholder | Real asset needed | Where it plugs in |
|---|---|---|---|
| Protagonist/NPC visuals | Default Unreal mannequin or a primitive-mesh Blueprint | MetaHuman (or equivalent) skeletal mesh + retargeted anim BP | `UATRCharacterData::CharacterPawnClass` |
| Facial performance | None | MetaHuman facial rig + Audio2Face/Live Link Face or authored facial anim | Hook via the character's Animation Blueprint; `Camera/`, `Cinematics/` already assume a face exists to shoot |
| Environments | Grey-box geometry / a single café room | Full art-directed environments, lighting, props | `Content/Environments/Placeholder/` → real level art |
| Animation | Default UE mannequin anim set | Full locomotion, gesture, and conversation animation set | Animation Blueprints referenced by character Blueprints |
| Audio (VO, music, SFX) | Silence / engine defaults | Licensed or original VO, music per mood state (Normal/Romantic/Tense/Mystery/Sad/Danger), SFX | `Content/Audio/Placeholder/` → Sound Cues/MetaSounds; music-state switching is a thin system not yet built — add a `UAudioMoodSubsystem` when music assets exist |
| Cinematics | None authored | Actual Level Sequences per story beat | `ACinematicTriggerActor::Sequence` |
| UI visual design | None (functional UMG only, not yet built) | Final UI art/skin | `Content/UI/` |

## Explicitly NOT done yet (build these next, tracked in `ROADMAP.md`)
- The Dialogue widgets themselves (`WBP_Dialogue`, `WBP_DialogueResponseButton`) — fully
  specified in `Content/UI/Dialogue/DialogueUMG_SPEC.md` and `SETUP.md` → "Dialogue UMG
  Setup," but not yet built (binary editor assets, can't be authored as files). This was
  the last remaining blocker for a playable café demo; everything it needs from C++ is
  now in place.
- UMG widgets for Main Menu, Pause, Inventory, Evidence Board, Objectives, Settings —
  the subsystems they'd bind to (`OnInventoryChanged`, `OnEvidenceDiscovered`,
  `OnQuestStateChanged`, etc.) already exist and are the intended data source; the
  Dialogue widget above is the only one needed for the café vertical slice milestone.
- The per-quest registration loop that runs once at session start
  (`DEVELOPMENT.md` → "Add a quest", step 2) — depends on how content is organized into
  chapters, which isn't decided yet. (Quest state save/restore itself is now implemented.)
- Mobile touch-control UMG widget (input plumbing on the C++ side is ready — see
  `MOBILE.md`).
- Music-state switching system referenced above.
- `USaveSubsystem::GetAvailableSaveSlots()` — platform-specific slot enumeration, still an
  unimplemented stub.
- At least one real `ULevelSequence` + a placed `ACinematicTriggerActor` to exercise the
  cinematic hook (`UCinematicSubsystem`) end-to-end; at least one shot camera actor
  assigned to a `UATRDialogueCameraComponent::ShotCameraActors` entry to exercise the
  camera hook. Both hooks are implemented and will log a clear warning instead of faking
  success until these are placed.
- Localization: `Content/Localization/` folder not yet created; all `FText` fields are
  already localization-ready (no raw `FString` in player-facing dialogue) so turning on
  the Localization Dashboard later requires no text-handling rework.
- Analytics hooks: no telemetry SDK integrated. `UStoryFlagSubsystem::OnStoryFlagChanged`,
  `UQuestSubsystem::OnQuestStateChanged`/`OnObjectiveCompleted`, and
  `URelationshipComponent::OnAxisChanged` are natural attachment points for an analytics
  subsystem once a provider (e.g. a first-party or third-party SDK) is chosen —
  deliberately not guessed at here.
