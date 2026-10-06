# Profession Gear Progression — ACTIVE CONTEXT

**Last updated:** 2026-10-06 (doc cleanup)
**Purpose:** live handoff file for agents working on this repo. Everything older (2026-10-02/03 turn updates, resume
audits, repo split, pre-install passes, m21 operator notes) is done and was removed; read it with
`git log -p ACTIVE_CONTEXT.md`. Design: `C:\KenshiModding\PROFESSION_GEAR_PROGRESSION_MOD_CONTEXT.md`, `ROADMAP.md`,
`DESIGN_IMPLEMENTATION_AUDIT.md`.

## CURRENT STATE — 2026-10-06

- Installed: ProfessionGearProgression.dll **BAFB8C31** (Normal / Normal rules) on the 5090 and the 4080.
- Every automated TEST_PLAN row is PASS (offline or live); open rows (Shay feel/tooltips, D4 athletics feel, deferred
  FCS content/stacking) in `INGAME_STATUS.md`, which also has the balance state and still-valid findings.
- Balance: gates + matrices for every profession ran (m28-m47); Shay decisions D1-D8 applied (farm factor 0.5,
  medic gear raises kit quality, Perception back on gear, crossbow fixed, turrets accepted, 132 vanilla crash).
- Engineering to-do: remove the no-op `HookLockpick` (hooks a dead KenshiLib stub; see INGAME_STATUS.md).
- Workflow: the coordinator (or a rig operator it names) runs the game on the 5090/4080; this repo's agent builds,
  tests offline, writes scenarios (`tests/ingame/`, `RUN_ORDER.md`), commits as `shaylanger <shaylanger2@gmail.com>`
  and pushes after each commit. `JobOperateScaling` is ON by default (jobs read the raw skill otherwise).

## Test tooling

- **Test configs:** `set_test_mode.ps1 -Mode Forced|Normal|NormalVerbose|AutoClassifyOff|Disabled
  -Rules InGameTest|Normal -Sidecar StartupTest|Empty|Restore` (fixtures in `tests/fixtures/`; the
  InGameTest rules turn plain vanilla backpacks into the specialist packs, exclude Straw Hat, tag
  Leather Vest as cooking, tag the UWE Medium Backpack as hauling).
- **Harness test commands registered by PG (TEST ONLY):** pg_info, pg_force_affix, pg_clear,
  pg_roll, pg_bonus (now prints base / vanilla_effective / effective / expected / match=1),
  pg_check (record obeys tier/cap/range/pool), pg_shop (scan + list a trader's shop storage),
  pg_building, pg_take, pg_store (move item instances building <-> character), pg_loot (move worn
  or body items between characters), pg_pack (backpack vanilla vs hooked weight, ratios),
  pg_census [name|faction filter] (records by owner class and stat).
