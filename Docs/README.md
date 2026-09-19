# After the Rain (working title)

A cinematic, story-driven interactive romance/mystery drama for Unreal Engine 5,
targeting Windows/Mac/Linux PC, Android, iOS, and (where practical) web.

This repository contains the **initial technical foundation**: a hybrid C++ + data-driven
architecture for dialogue, relationships, quests, investigation, inventory, cinematics,
save/load, and a working demo scene proving the pipeline end-to-end. It intentionally does
**not** contain final art, animation, VO, or music — see `ASSET_PIPELINE.md` for exactly
what's placeholder and what needs to be supplied.

## What's here vs. what needs Unreal Editor
Everything under `Source/` is compilable C++. Everything under `Content/` that ends in
`.md` is a **spec for a binary editor asset** (DataTable, Data Asset, Level, Level
Sequence) that cannot be authored as a text file — Unreal doesn't have a text format for
these. Each spec gives exact field values so creating the real asset in-editor is a
copy-paste job, not a design task. See `SETUP.md` for the first-run checklist.

## Quick links
- `ARCHITECTURE.md` — system map and how systems talk to each other
- `SETUP.md` — opening and first-run configuration
- `DEVELOPMENT.md` — how to add a character, a conversation, a choice, a quest, a cinematic
- `GAME_SYSTEMS.md` — deeper reference for each gameplay system
- `ASSET_PIPELINE.md` — placeholders and exactly what real assets must replace them
- `MOBILE.md` — Android/iOS-specific notes
- `DEBUGGING.md` — developer tools and console commands
- `ROADMAP.md` — recommended next development phases

## Project identity
- Working title: **After the Rain** — change freely; it only appears in `DefaultGame.ini`
  (`ProjectDisplayedTitle`) and this doc set.
- Genre: cinematic interactive romance drama, with mystery/investigation and relationship-sim mechanics.
- Original IP: no characters, dialogue, or assets from any existing game. "Sarah" and "the café"
  in the demo scene are original placeholder content for this project only.
