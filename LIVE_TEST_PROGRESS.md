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
