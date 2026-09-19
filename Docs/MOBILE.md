# Mobile (Android / iOS) Notes

## What's already mobile-aware
- Input is Enhanced Input throughout (`ATRPlayerCharacter`), so touch controls are an
  additional *input source* bound to the same `UInputAction`s — no gameplay branching
  on platform (see `DEVELOPMENT.md` → "Add mobile touch controls").
- `UInteractionComponent` uses a forward sweep from camera position, which works
  identically whether "forward" comes from a gamepad stick, mouse, or a fixed touch
  reticle in the center of the screen — no separate mobile interaction path needed.
- `DefaultEngine.ini` sets Vulkan/ES3.1 for Android and Metal/iOS 15+ minimum as sane
  defaults; ray tracing is off by default (mobile GPUs don't support it) with software
  Lumen as the indirect-lighting fallback.

## Still needed for mobile (tracked in `ASSET_PIPELINE.md` / `ROADMAP.md`)
- On-screen virtual joystick + interact/menu buttons (UMG widget).
- Mobile-specific UI scaling (`UI/` system not yet built — design for variable safe-area
  and aspect ratio from the start rather than retrofitting).
- Quality presets: Unreal's Scalability system (`sg.*` CVars) should be exposed via the
  Settings screen once built; no mobile-specific quality tier decisions have been made yet.
- Packaging: Android requires a signed keystore (Project Settings → Platforms → Android)
  and a Google Play target API level meeting current Play Store requirements at time of
  submission — check Google's current minimum before submitting, as these change yearly.
- iOS requires an Apple Developer account, provisioning profile, and App Store Connect
  entry — standard Unreal iOS packaging flow, no project-specific deviation needed.

## Performance
No mobile-specific LOD/streaming budget has been set (that requires real assets to
measure against). `Source/` has no per-frame allocations in hot paths (`UInteractionComponent`
ticks at 10Hz via `TickInterval`, not every frame) as a starting discipline, but a real
profiling pass belongs in the "Content & Production" roadmap phase once art exists.
