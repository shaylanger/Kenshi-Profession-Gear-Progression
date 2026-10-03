# Profession Gear Progression — Live Test Progress

**Last updated:** 2026-10-02
**Current build state:** live testing authorized; ProfessionGearProgression installed in D: Kenshi mods; forced test config selected.
**Canonical technical plan:** TEST_PLAN.md (through test 307)
**Player/phase plan:** LIVE_TEST_RUNBOOK.md

## Resume instructions for another agent

1. Read this file, ACTIVE_CONTEXT.md, LIVE_TEST_RUNBOOK.md, and recent git diff/log.
2. Check Kenshi with `C:\KenshiModding\tools\automation\kenshi-ctl.ps1 status`.
3. Do not repeat PASSED phases unless a code change invalidates them.
4. After every automated phase or meaningful finding, update this file **before** continuing.
5. Archive logs/artifacts on failures before restarting Kenshi.

## Current machine state at recovery after interrupted run

- Kenshi process: **not running**.
- Steam: running.
- Stobe test inbox: enabled.
- ProfessionGearProgression package: installed at `D:\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression`.
- `data\mods.cfg`: contains `ProfessionGearProgression.mod` at end of current load order.
- Installed config: forced-test config (VerboseLogging=true, all source multipliers forced high).
- ProfessionGear.log: not yet created.
- profession_gear_affixes.tsv: not yet created.
- Conclusion: **no live ProfessionGear phase had actually run yet** before the interruption.

## Recovered uncommitted loader fix

Interrupted work had changed the plugin entry from plain C export to the C++-mangled RE_Kenshi contract:
- source: `__declspec(dllexport) void startPlugin()`
- expected export: `?startPlugin@@YAXXZ`
- Stobe.dll uses the same export.
- installed ProfessionGear DLL currently also exposes `?startPlugin@@YAXXZ`.
- TEST_PLAN test 307 records this loader contract.

These changes must be validated/committed before moving past startup testing.

## Phase status

| Phase | Status | Notes |
|---|---|---|
| Pre-live recovery / loader contract | IN PROGRESS | Validate uncommitted test 307 + rebuild/package/install consistency |
| 1 Startup | NOT RUN |
| 2 Equip-only + tooltip | NOT RUN |
| 3 Crafting | NOT RUN |
| 4 NPC context | NOT RUN |
| 5 Vendors | NOT RUN |
| 6 Exploration/world loot | NOT RUN |
| 7 Backpacks/hauling | NOT RUN |
| 8 Real job-path proof | NOT RUN |
| 9 Utility proof | NOT RUN |
| 10 Normal rarity/distribution | NOT RUN |
| 11 Compatibility | NOT RUN |
| 12 Balance | NOT RUN |

## Latest evidence

- Stobe export: `?startPlugin@@YAXXZ`.
- Installed ProfessionGear export: `?startPlugin@@YAXXZ`.
- No ProfessionGear runtime log yet, so startup/load behavior remains unproven.

## Next exact action

Run offline core/build/SDK/package validation with the recovered loader change, reinstall the freshly packaged build if needed, commit the recovery checkpoint, then launch Kenshi through `kenshi-ctl.ps1` and execute Phase 1.


### Recovery checkpoint completed

- Recovered RE_Kenshi entry-point fix validated offline.
- Fresh package rebuilt and reinstalled.
- Forced test config reapplied.
- Commit: `559e6e8 fix: use RE_Kenshi plugin entry contract`.
- Git pushed to `origin/main`.
- Next action: launch `auto-home` through the Kenshi automation harness and begin Phase 1 startup validation.


### Phase 1 startup — partial PASS checkpoint

Launch: `auto-home` via `kenshi-ctl.ps1 launch -Save auto-home`.

Observed:
- Kenshi reached game process successfully (`RE_Kenshi\kenshi_x64.exe --norestart`).
- Harness health: **ok**.
- ProfessionGear runtime log created.
- Startup version: `0.9.0-pretest`.
- Forced config loaded exactly:
  - enabled=1
  - autoClassify=1
  - verboseLogging=1
  - globalChance=100
  - all source multipliers=1
  - maxAffixes=3
- All 12 runtime hooks report `hooked`:
  - PlayerInterface::update
  - CharStats::getStat
  - CraftingBuilding::addFinishedCraftItem
  - Inventory::getTotalWeight
  - Inventory add/remove/update callbacks
  - tooltip base/Armour/ContainerItem/Crossbow/Sword
- No ProfessionGear crash/hang on startup.
- Sidecar not yet present at this checkpoint; fixture/world load not yet confirmed complete.
- Stobe status at this exact check still reported a loading state, so Phase 1 is not yet marked complete.

Next: wait for world-ready, exercise pause/speed/state commands, inspect runtime log/sidecar again.


### Phase 1 live scan — STARTUP PASS / CLASSIFIER FAILURE FOUND

World reached `phase=world`; pause/unpause and 1x/3x speed controls worked; harness health remained `ok`.

ProfessionGear then generated a sidecar successfully, proving runtime scanning/persistence is active.

**Live defect discovered from verbose generation log:**
Non-equippable inventory/resources were being classified as profession gear, including:
- `Chewing Tobacco` -> WORKWEAR_GENERIC
- `Bolts [Regulars]` -> WORKWEAR_GENERIC
- `Basic First Aid Kit` / `Standard First Aid Kit` / `Splint Kit` -> TOOL_MEDIC
- `Medical Supplies` -> TOOL_RESEARCH / TOOL_MEDIC

This violates the approved equip-only design: items that cannot be equipped must not receive ProfessionGear affixes/tooltips merely because their name/description contains semantic words.

**Disposition:** Phase 1 startup/hook mechanics pass, but the current build is rejected for further testing until an equipment-class eligibility gate is added. Current generated sidecar is contaminated test data and must be discarded after the fix.

Next:
1. stop Kenshi and archive current artifacts,
2. require item to be an actual Gear-derived/equippable class before classification,
3. add regression fixtures for the exact live false positives,
4. rebuild/reinstall forced test build,
5. delete contaminated sidecar and rerun Phase 1.
