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


### Non-equippable classifier fix checkpoint

- Added explicit runtime eligibility: only `Gear` subclasses or `ContainerItem` may create ProfessionGear records.
- Non-equippable consumables/resources now fail before persistence/tooltips.
- Added tests 308–311 from live false positives.
- Offline suite now: **5,217 checks PASS**, build OK, 17/17 symbols, 3/3 source contracts, package verified.
- Fresh package installed; forced config restored.
- Contaminated ProfessionGear sidecar/log from failed first scan deleted.
- Commit/push: `f1517ac fix: reject non-equippable profession items`.
- Next: rerun Phase 1 and verify Chewing Tobacco, Bolts, First Aid/Splint kits and Medical Supplies produce no ProfessionGear records.


### Phase 1 corrected-build rerun — PASS

Corrected build `f1517ac` launched into the same fixture successfully.

Live verification:
- world reached `phase=world`,
- all 12 ProfessionGear hooks loaded,
- harness health remained OK,
- previous live false positives are gone:
  - no Chewing Tobacco record,
  - no Bolts [Regulars] record,
  - no First Aid/Splint Kit record,
  - no Medical Supplies record.
- fresh sidecar contains only 1 eligible record plus header at this checkpoint:
  - `Square Goggles` -> `GOGGLES_GENERIC`, tier 1, Turrets +3%.
- persistence file generation works.
- startup/load/pause/speed behavior had already passed and remained stable after the rebuild.

**Phase 1 status: PASS on corrected build.**
Tests 308–311 also pass live for the observed fixture sample.

Next: Phase 2 automation — determine available inventory/equip/stat harness commands, then exercise carried vs equipped behavior and persistence.


### Phase 2 live failure — equipped items missing from current cache scan

Controlled test:
- Square Goggles persisted roll: Turrets +3%.
- Set Shay Turrets base to 50.
- Before equipping, effective Turrets = 46.1 (existing vanilla gear penalty/interaction).
- Harness `equip Shay "Square Goggles"` succeeded and reported section=belt, equipped=1.
- Immediately after equip:
  - harness `iteminfo` could no longer find Square Goggles via `Inventory::getAllItems()`,
  - ProfessionGear effective Turrets remained 46.1 instead of increasing.

Root-cause hypothesis strongly supported:
- Stobe harness `FindInventoryItem` scans `Inventory::getAllItems()`.
- ProfessionGear `RebuildCharacterBonusCache` also scans `Inventory::getAllItems()`.
- In live Kenshi, equipped belt gear is no longer in that collection.
- Therefore ProfessionGear likely misses equipped section items entirely and cannot activate their affixes.

**Phase 2 status: FAILED / implementation fix required.**

Next:
1. inspect Inventory section APIs and live section names,
2. rebuild cache by scanning actual equipped sections/items,
3. add regression/live tests,
4. update harness item lookup if needed for automated unequip,
5. rebuild/reinstall and rerun Phase 2.


### Equipped-section fix build/install checkpoint

ProfessionGear changes prepared:
- added `CollectCharacterInventoryItems()`,
- scans every `InventorySection` via `getAllSections()` + `section->getItems()`,
- falls back to `getAllItems()` defensively,
- `ProcessCharacter()` now generates records for top-level items across main + equipped sections,
- `RebuildCharacterBonusCache()` now sees belt/armour/weapon/backpack equipped items.

Stobe harness patch prepared:
- `/root/STOBE-src/src/TestAutomation.cpp::FindInventoryItem` now scans all inventory sections instead of only `getAllItems()`,
- this is required so automated `iteminfo/equip/unequip` can find gear after equip.

Important repo note:
- `/root/STOBE-src` currently contains many unrelated modified/untracked files from ongoing STOBE work.
- Do **not** bulk-stage/commit that repo for this harness patch.
- TestAutomation.cpp is currently untracked in that repo; preserve it as part of the active test harness state.
- Installed Stobe test DLL after this patch: **BE903ED2**.

Build/install:
- Stobe build: BUILD OK.
- ProfessionGear core: 5,217 checks PASS.
- ProfessionGear DLL: BUILD OK.
- SDK/export verification: 17/17 PASS.
- Package verification: PASS.
- Fresh ProfessionGear package installed with forced config.
- ProfessionGear sidecar/log absent before next launch (clean run).

Next exact action: launch auto-home and rerun Square Goggles Turrets base/effective -> equip -> effective -> unequip -> effective, plus verify already-equipped Ninja Rags/Wooden Sandals now generate records.


### Phase 2 equipped-section rerun — core equip-only PASS

Corrected section-scanning build + Stobe harness patch live results:

Fixture evidence:
- already-equipped `Wooden Sandals (colored)` generated record while equipped:
  - tags BOOTS_TRAVEL
  - Perception +18.6%, Athletics +23.2%
- already-equipped `Ninja Rags` generated record while equipped:
  - tags STEALTH_GEAR, THIEF_GEAR
  - Stealth +5.8%
- previous non-equippable false positives remained absent.
- confirms test 312 discovery path is working.

Controlled Square Goggles test:
- persisted roll in this run: **Engineering +5.3%**
- item before equip: section=main, equipped=0
- set Shay Engineering base to 50.0
- unequipped effective Engineering: **20.7** (vanilla/mod equipment penalties already applied)
- equip command succeeded; item moved to section=belt, equipped=1
- harness `iteminfo` still found it in equipped section (test 313 PASS)
- equipped effective Engineering: **21.8**
- 20.7 × 1.053 = 21.7971, matching observed 21.8
- unequip succeeded
- effective Engineering returned exactly to **20.7**

Conclusions:
- ProfessionGear percentage composes on top of Kenshi's existing effective value.
- carrying/unequipped item gives zero ProfessionGear bonus.
- equipping applies exactly one expected bonus.
- unequipping removes it.
- tests 62–80 core equip-only path PASS for Engineering sample.
- tests 312–313 PASS live.

Phase 2 remaining before full pass:
- save/reload exact roll/identity,
- tooltip visual duplicate/readability is still visual-only unless screenshot automation can confirm it.


### Harness nuance during Phase 2 persistence setup

After the successful equip -> unequip test:
- `unequip Shay "Square Goggles"` set `equipped=0`,
- but the item remained in inventory section `belt`,
- a subsequent `equip` call failed while it was still in the belt section.

This appears to be a Stobe test-harness inventory-placement nuance, not a ProfessionGear bonus failure:
- ProfessionGear correctly removed the bonus when `isEquipped` became false.
- For persistence testing, reload a fresh `auto-home` fixture so the goggles begin in `main`, then equip once and save while equipped.

Potential harness improvement later: make `unequip` move the item back to a valid non-equip section, or add a `moveitem` helper.


### Phase 2 persistence — HARD FAIL confirmed

Exact same-save reload test:
- before loading `pg-phase2-equipped`:
  - Square Goggles handle = `53665-2180541952-0-0-3`
  - equipped=1
- after loading the exact save `pg-phase2-equipped`:
  - Square Goggles handle = `57407-1135864320-0-0-3`
  - equipped=1
- Engineering base remained 50 from the saved state.
- ProfessionGear Engineering bonus did **not** survive; effective remained 20.7.

Conclusion:
- Kenshi runtime `hand::toString()` is not stable across ordinary save/reload.
- Current sidecar keying by runtime hand is invalid for persistence.
- Test 123 (save/reload same save) FAILS.
- Tests 124/131/132 are blocked until item identity is redesigned.
- Do not proceed to later gameplay/balance phases until persistence is fixed.

Next investigation:
1. instrument harness item info with raw hand fields,
2. inspect `Item::persistant`, proper owner, and inventory-owner handles across reload,
3. determine whether any built-in serialized identity survives,
4. if none does, implement sidecar reconciliation based on stable item fingerprint + owner/section/position metadata or hook serialization to persist a custom ID.


### Persistence v2 implementation checkpoint

Implemented save-native item identity:
- added per-item `ProfessionGearPersistentId`,
- runtime map binds volatile Kenshi handle -> persistent ID,
- sidecar records now use persistent ID as `instanceKey`,
- hooked `Item::serialiseInInventory` to write the persistent ID into item `GameData.sdata`,
- hooked `Item::loadFromSerialiseInInventory` to restore/bind it after reload,
- load without a stored ID clears any stale runtime binding,
- persistent-ID/base-item collisions create a new ID rather than stealing/deleting another item's record,
- pre-v2 runtime-handle sidecar rows are ignored.

Added tests 314-319.

Offline validation:
- core: **5,217 PASS**
- DLL: BUILD OK
- SDK/export: **19/19 PASS**
- source contracts: **6/6 PASS**
- package verification: PASS

Next exact test:
1. install clean v2 package + forced config,
2. delete old v1 sidecar/log,
3. load fresh auto-home,
4. use Square Goggles (or another eligible item), record persistent ID/affix,
5. equip and save to a new v2 save,
6. reload exact save and confirm runtime handle changes but persistent ID/affix/effective stat remain,
7. fully restart Kenshi and repeat.
