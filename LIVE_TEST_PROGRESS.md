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


## Recovery checkpoint — 2026-10-02 ~20:16

The user hit a UI timeout/retry while the automated live run continued in the background.

### Current runtime state
- Kenshi: **running and healthy**, PID 44280, RE_Kenshi child process.
- Current harness state: `phase=world save=pg-persist-v2 paused=0 speed=1.0 squad=2 player=Shay`.
- Shay and Malzin are present.
- ProfessionGear startup succeeded.
- All expected hooks reported `hooked`, including:
  - PlayerInterface update
  - CharStats getStat
  - Crafting finished-item
  - Inventory total weight
  - Inventory add/remove/update
  - Item serialise/load identity hooks
  - tooltip base/Armour/Container/Crossbow/Sword
- Forced-test config is active.
- ProfessionGear log exists and is actively writing.
- Sidecar exists and currently contains ~930 generated records.
- No crash/hang detected.

### Harness history discovered
- Stobe frame listener started.
- `auto-home` autoloaded successfully.
- Later the harness loaded a dedicated save named `pg-persist-v2`.
- Therefore the interrupted automation already executed additional ProfessionGear test setup after Phase-1 launch.

### Immediate next step
Reconstruct ProfessionGear-specific harness commands and results from Stobe/ProfessionGear logs before issuing new commands. Do not repeat already-passed persistence/setup work until its evidence is classified.


### Persistence v2 live verification — in-process reload PASS

Current `pg-persist-v2` evidence:
- Shay's equipped Square Goggles runtime handle after reload: `57374-2915527424-0-0-3`.
- ProfessionGear restore log bound that volatile handle to persistent ID:
  `pgp1-44280-278351953-1-585157587`.
- The same persistent ID originally rolled:
  - base: `2168-gamedata.base`
  - item: Square Goggles
  - tier: 1
  - affix: **Science +5.3%**
- Sidecar contains that persistent-ID record.
- The runtime handle is different from the pre-save handle, proving the lookup is no longer relying on volatile `hand::toString()`.
- Equipped Science remains above the same vanilla-effective baseline after reload.

Status:
- test 314 PASS (ID serialized)
- test 315 PASS (ID restored/bound)
- test 316 PASS (runtime handle changed but same persistent record resolved)
- test 317 PASS (equipped affix active after same-save reload)
- test 318 NEXT: full Kenshi process restart + reload.


### Phase 1 — automated startup PASS

Evidence:
- Kenshi launches and remains healthy under RE_Kenshi.
- ProfessionGear starts with forced config.
- All expected hooks report success.
- Stobe frame listener/autoload works.
- save/load transitions complete.
- high/normal/pause speed commands already occurred without crash.
- sidecar writes successfully.

Phase 1 status: **PASS (AUTO)**.
Visual-only tooltip readability remains part of Phase 2.

### Phase 2 — equip-only + persistence core PASS

Post-restart controlled Science sample using Square Goggles:
- persistent ID: `pgp1-44280-278351953-1-585157587`
- affix: **Science +5.3%**
- equipped Science: **22.0**
- after unequip: **20.9**
- after reloading `pg-persist-v2`: **22.0**
- runtime handle changed on reload again, but restored persistent ID/affix remained.

Results:
- carrying/unequipped bonus = 0
- equipped bonus applies exactly once
- unequip removes it
- same-save reload restores exact affix
- full Kenshi process restart restores exact affix
- tests 314–318 PASS live
- tests 62–80 core equip-only behavior PASS for this sample
- tests 312–313 PASS
- original test 123 failure from v1 identity design is superseded by v2 and now PASS

Phase 2 status: **PASS (AUTO CORE)**.
Remaining Phase-2-only item: tooltip visual/readability/duplicate confirmation (SHAY/visual unless screenshot automation is sufficient).

### Live non-equippable regression

Forced-mode fresh-item tests:
- **308 PASS** — Chewing Tobacco added to Shay; ProfessionGear log +0, sidecar +0.
- **310 PASS** — Basic First Aid Kit added to Shay; ProfessionGear log +0, sidecar +0.
- **311 PASS** — Medical Supplies added to Shay; ProfessionGear log +0, sidecar +0.
- **309 PENDING** — Bolts [Regulars] command was swallowed by nested-shell quoting (`no data named: Bolts`), so this is harness invocation failure, not mod failure.

Conclusion so far: non-Gear consumable/medical/trade items are correctly rejected before record generation under forced 100% eligibility mode.

### Harness improvement planned for deterministic item tests

Problem: `stobe-auto give/find` exact-matches display names, but the current UWE-heavy mod stack has many near-duplicate names (e.g. Rattan Hat variants), causing ambiguous test setup.

Planned harness-only fix:
- `FindData(...)` should first exact-match Kenshi `GameData::stringID`, then exact display name, then substring fallback.
- This does not change gameplay behavior; it only makes automation deterministic.
- After rebuild/install, remaining ProfessionGear tests can target known IDs such as `2185-gamedata.base` or `2168-gamedata.base` directly.

### Harness deterministic item-ID support installed

- Patched only `/root/STOBE-src/src/TestAutomation.cpp`.
- `FindData(...)` now exact-matches `GameData::stringID` before display-name matching.
- Rebuilt Stobe successfully.
- Installed Stobe hash changed: `CE55092D -> 89045BBE`.
- No unrelated STOBE files were intentionally edited by this test work.
- Purpose: deterministic ProfessionGear automation in a mod stack with many duplicate/variant display names.

### Exact-ID harness + world-loot live results

Harness exact `GameData::stringID` lookup is confirmed working in live Kenshi.

Results:
- **309 PASS** — `95781-rebirth.mod` Bolts [Regulars] added to Shay under forced mode: ProfessionGear log +0, sidecar +0.
- **241 PASS** — fresh eligible item injected to player is treated as `world_loot`, not player profession context.
- **242 PASS** — `2185-gamedata.base` Rattan Hat generated `source=world_loot`, tag `WORKWEAR_GENERIC`, affix `Medic +22.7%`; exactly one new targeted record.
- **243 PASS** — `2168-gamedata.base` Square Goggles generated `source=world_loot`, tag `GOGGLES_GENERIC`, affixes `Robotics +17.6%, Engineering +17.8%, Turrets +16.7%`; all are inside the legal goggles pool.

Note: background streamed NPCs generated unrelated verbose lines during the goggles test, but the target world-loot record is unambiguous by base ID/source and sidecar identity.

### Context-test nuance: player squad items intentionally use world-loot source

Attempted role-bias setup by setting Malzin to Farming/Medic/Science 100 and injecting generic gear.
Observed source remained `world_loot` because Malzin is a player character.
This is expected from the current anti-player-bias rule: any previously unseen non-crafted item first observed on a player character clears role context and becomes world loot.

This does **not** test NPC role bias. Tests 93–100 / 235–236 must use spawned non-player NPCs. No pass/fail assigned to those rows yet.

### NPC context live finding — generic multi-affix coherence BUG

Spawned non-player `Farmer` (Slowline #1499145472), forced Farming to 100, then added the exact same generic `2185-gamedata.base` Rattan Hat.

Observed target roll:
- source: `npc`
- tags: `WORKWEAR_GENERIC`
- tier: 6
- affixes: `Farming +16.3%, Armour Smithing +19.5%, Robotics +20.2%`

What passed:
- role bias worked: first affix correctly followed Farming.

What failed:
- top-tier second/third affixes came from the entire broad generic-workwear pool, creating an incoherent Farmer item.

Decision/fix required before further role testing:
- when contextual generic gear has a matching NPC role, narrow the allowed roll pool to that role for that instance (or a deliberately related pool), so Farmer workwear stays Farming-focused.
- trader/world-loot generic gear retains the broad random pool because it has no wearer role yet.

Tests 93/235: **FAIL on coherence in current build; fix in progress**.

### Contextual generic coherence fix — LIVE PASS

After rebuilding/reinstalling the narrowed-pool fix:

1. Spawned non-player Farmer `Barik`, Farming forced to 100.
   - exact item: `2185-gamedata.base` Rattan Hat
   - source: `npc`
   - tags: `WORKWEAR_GENERIC`
   - tier: 6
   - affixes: **Farming +22.0% only**

2. Spawned non-player Tech Hunter Researcher `Stinks`, Science forced to 100.
   - exact item: `2168-gamedata.base` Square Goggles
   - source: `npc`
   - tags: `GOGGLES_GENERIC`
   - tier: 6
   - affixes: **Science +17.3% only**

Results:
- test 93 / generic farmer role path PASS
- test 95 / researcher generic-goggle role path PASS
- test 235 PASS for known-role workwear coherence
- test 236 PASS for known-role goggles coherence
- test 320 PASS live

Trader/world-loot generic gear still keeps the broad legal pool when there is no wearer profession context.
