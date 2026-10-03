# Profession Gear Progression — Live Test Progress

**Last updated:** 2026-10-02
**Current build state:** live testing authorized; ProfessionGearProgression installed in D: Kenshi mods; forced test config selected.
**Canonical technical plan:** TEST_PLAN.md (through test 307)
**Player/phase plan:** LIVE_TEST_RUNBOOK.md

> **Harness moved (2026-10-02):** the test commands used below (`stobe-auto iteminfo/equip/unequip/
> traders/transfer/packput/packweight/craftfinish/...`) no longer live in Stobe's `TestAutomation.cpp`.
> They are in the standalone Kenshi Automation Harness mod (`C:\KenshiModding\Kenshi-Automation-Harness`,
> `src/Commands.cpp`; reference `docs/COMMANDS.md`, agent notes `AGENTS.md`). `stobe-auto` works
> as before (it now wraps the harness client) and needs the harness on (`stobe-say on`). Add new test
> commands in the harness repo, or register ProfessionGear-specific ones from the ProfessionGear plugin
> through the harness extension API (`include/KenshiAutomationHarness.h`, `docs/EXTENDING.md`).
> Mentions of `TestAutomation.cpp` further down are history.
> **Since 2026-10-02 (late):** the plugin registers `pg_info`, `pg_force_affix <npc> <item> <stat> <pct>
> [... ] [tier n]`, `pg_clear <npc> [item]`, `pg_roll <npc> <item>`, `pg_bonus <npc> <stat>` (validated in
> game); the harness has `craft <npc> <item> at <bench>` (real crafting, `research` to unlock),
> `craftfinish` (fixed: it really completes a craft now), `benches`, `job`, `fill`, `time`/`wait-game`
> and the `kah run` scenario runner. Shay's "Crafting base" save (benches in the Hub) unblocks
> Phase 3 / tests 85-92 and 306. Fixture details: `C:\KenshiTestFixtures\FIXTURES.md` (local).

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

### Additional NPC role-context live passes

- **97 PASS (Medic context):** spawned `Doctor /GENNAME/` (Doctor Colby), Medic forced to 100, exact generic Rattan Hat generated `source=npc`, `WORKWEAR_GENERIC`, **Medic +20.4% only**.
  - Harness `give` reported 0/1 because the hat auto-equipped instead of remaining in main inventory; ProfessionGear still saw the item and rolled it correctly.
- **96 PASS (Engineering context):** spawned `Engineer /GENNAME/` (Engineer Double), Engineering forced to 100, exact generic Rattan Hat generated **Engineering +21.1% only**.

Known-role generic workwear coherence is now confirmed live across Farming, Medic, Engineering; generic goggles confirmed with Science.

### Trader-stock synthetic-template test — INCONCLUSIVE, not a mod failure

Spawned `Trader /GENNAME/` (`Pethra [Trader Stelania]`) and forced Medic 100. Added five unequipped exact `1168-gamedata.base` Cloth Shirts.

Observed all five targeted rolls:
- `source=npc` (NOT `source=trader`)
- `equipped=0`
- all narrowed to Medic

Interpretation:
- the spawned trader-looking template is **not** returning Kenshi `Character::isATrader()==true` in this runtime state.
- therefore this setup tests ordinary NPC inventory, not real shop stock.
- tests 211/219/237 remain PENDING rather than FAIL.

Next action: add a harness-only `traders [radius]` query that lists characters for whom Kenshi itself reports `isATrader()==true`, then target a real live merchant.

### Harness trader-discovery helper added

Added harness-only command `traders [radius]` in Stobe TestAutomation. It lists only characters for whom Kenshi runtime `Character::isATrader()` returns true. Purpose: distinguish real merchant/shop actors from NPC templates merely named Trader.

### Real trader-stock context — PASS

Used harness `traders` command to locate actual runtime merchants (`Character::isATrader()==true`). Teleported Shay near real Shinobi trader **Blamo**.

Test setup:
- Blamo Medic forced to 100.
- Blamo already had occupied/equipped clothing, so five exact `1168-gamedata.base` Cloth Shirts remained unequipped.

Observed all five target rolls:
- `source=trader`
- `equipped=0`
- varied legal generic-workwear affixes:
  - Cooking + Robotics
  - Farming + Medic
  - Crossbow Smithing + Science
  - Cooking + Crossbow Smithing
  - Robotics
- rolls were **not** collapsed to Blamo's Medic role.

Results:
- **211 PASS** — generic workwear in true trader inventory can roll random legal profession stock.
- trader source detection (`isATrader`) is functioning live.
- synthetic `Trader /GENNAME/` test was correctly inconclusive because that spawned template was not runtime-flagged as trader.
- **219 still PARTIAL** until actual naturally generated merchant stock (not harness-injected inventory) is confirmed to use the same character inventory/source path.

### Harness batch extension planned

To automate remaining Phase 5–7 tests without repeated rebuilds, add three test-only Stobe commands:
- `transfer <from> <to> <item>`: move the same unequipped Item* instance using Inventory remove-without-destroy + add; preserves ProfessionGear identity for purchase/loot transfer tests.
- `packput <npc> <pack> <item> [n]`: create exact cargo directly inside an owned ContainerItem.
- `packweight <npc> <pack>`: recalculate/read the backpack inventory total through the live ProfessionGear weight hook and report equipped state.

These are harness-only and do not change normal gameplay code.

### Harness batch commands implemented

Patched only Stobe `src/TestAutomation.cpp` with:
- exact `stringID` matching in inventory-item lookup,
- `transfer <from> <to> <item>` using `removeItemDontDestroy_returnsItem` + `addItem` to preserve the same Item* instance,
- `packput <npc> <pack> <item> [n]` for deterministic cargo setup,
- `packweight <npc> <pack>` reporting equipped state, raw contained-item weight, and hooked total weight.

Next: rebuild/install Stobe, then use these to automate purchase/loot identity and backpack weight tests.

### Trader item transfer / purchase-style identity — LIVE PASS

Real trader Blamo generated exact `1168-gamedata.base` Cloth Shirt:
- source `trader`
- persistent sidecar key `pgp1-45748-280821234-1-3937117084`
- affixes `Medic +16.2%, Engineering +17.4%`

Harness `transfer` moved the **same Item*** from Blamo to Shay:
- runtime handle before: `62156-3313232384-0-0-3`
- runtime handle after:  `62156-3313232384-0-0-3`
- no target reroll occurred.

Equip-only check on transferred item:
- carried/unequipped after freeing shirt slot: Medic 1.7, Engineering 1.9
- equipped transferred trader shirt: Medic 1.9, Engineering 2.3
- same handle remained equipped.

Results:
- **214 PASS at engine transfer level** — trader-generated affix survives same-instance merchant→player transfer.
- **215 PASS** — transferred purchased-style item gives no bonus while carried/unequipped.
- **216 PASS** — stored affix activates when equipped.
- actual trade-UI click flow is not separately automated; the critical item identity/transfer behavior is proven.

### Generic backpack live weight control — PASS

Used real equipped `Small Thieves Backpack` (base `1288-gamedata.base`) on Tengu.
- added exact Building Material cargo (`580-gamedata.base`)
- equipped: raw=2.0, total=2.0
- unequipped: raw=2.0, total=2.0

No ProfessionGear specialist weight reduction leaked onto the generic/thief backpack.
Results:
- **117 PASS** — generic backpack same contents unchanged by ProfessionGear specialist system.
- **299 PASS (weight half)** — generic pack receives no specialist contents-weight reduction.

### Final planned harness batch before next rebuild

Add:
- `CONTAINER` GameData support to `find/give/stash/buy/packput`, so exact backpack records can be created by stringID.
- `craftfinish <npc> <item name|stringID>`: locate nearest real `CraftingBuilding`, set `whosCrafting` to the chosen NPC, create the requested item, and call the real `addFinishedCraftItem()` on the game thread. Purpose: live-test ProfessionGear's finished-item craft hook and final-item roll path.

If `craftfinish` cannot find a real crafting building in the fixture, it will fail safely and Phase 3 remains pending rather than fabricating a pass.

### Final harness batch implemented

TestAutomation now additionally supports:
- `CONTAINER` records in find/give/stash/buy/packput data resolution,
- `craftfinish <npc> <item>` using nearest real `CraftingBuilding::addFinishedCraftItem()` on the game thread,
- existing transfer/packput/packweight commands from prior batch.

Source sanity check found no remaining 3-type item loops; all relevant arrays include ITEM/WEAPON/ARMOUR/CONTAINER.


### Live launch recovery checkpoint — after timeout

The previously timed-out harness launch **did actually run ProfessionGear in Kenshi**.

Evidence recovered:
- Kenshi is currently not running.
- `ProfessionGear.log` exists (~94 KB).
- `profession_gear_affixes.tsv` exists (~57 KB).
- The live log contains both persisted-record restores and new forced-mode rolls.
- Live examples observed:
  - Wooden Sandals -> BOOTS_TRAVEL -> Athletics + Perception rolls.
  - Rag Shirt / Rag Loincloth / Holy Servant Rags -> WORKWEAR_GENERIC -> contextual profession rolls such as Engineering or Medic.
  - Black Rag Shirt -> STEALTH_GEAR -> Stealth/Lockpicking.
  - a Staff instance classified TOOL_FARMING and rolled Farming; this needs classifier-evidence review before calling it valid.
- Installed build therefore passed the fundamental loader/startPlugin barrier and executed runtime hooks far enough to scan live inventories and write persistence.

Phase 1 is **PARTIAL / INVESTIGATING**, not yet PASS:
- startup/load execution proven,
- persistence write proven,
- need determine why Kenshi is no longer running (normal exit vs crash vs harness timeout/termination),
- need inspect startup hook lines and RE_Kenshi/crash evidence before relaunch.


### Recovery reconstruction — archived work after test 320

Recovered from committed LIVE_TEST_PROGRESS history plus archived ProfessionGear snapshots at:
- 20:36 `pre-traders-harness`
- 20:43 `pre-transfer-pack-harness`
- 20:51 `pre-container-craft-harness`

Do **not** rerun the following already-proven items unless later code changes invalidate them.

#### Confirmed PASS before timeout

Startup / persistence / equip:
- Phase 1 AUTO startup: PASS.
- Phase 2 AUTO core: PASS.
- tests 62–80 equip-only/effective-stat sample: PASS.
- 123 same-save persistence: PASS under persistent-ID v2.
- 308–311 non-equippable false-positive regressions: PASS.
- 312 equipped-section discovery: PASS.
- 313 harness lookup after equip: PASS.
- 314–318 persistent-ID serialize/restore/in-process/full-process reload: PASS.
- 319 legacy pre-v2 rows ignored by v2 loader design/offline checks.
- 320 contextual generic coherence: PASS live.

World loot:
- 241 PASS — player-first-seen eligible item uses world_loot source.
- 242 PASS — Rattan Hat world-loot roll stayed inside WORKWEAR_GENERIC pool.
- 243 PASS — Square Goggles world-loot roll stayed inside goggles legal pool.

NPC context:
- 93 PASS — Farmer generic workwear narrows to Farming only.
- 95 PASS — Researcher generic goggles narrows to Science only.
- 96 PASS — Engineer generic workwear narrows to Engineering only.
- 97 PASS — Doctor generic workwear narrows to Medic only.
- 235 PASS — known-role workwear coherence.
- 236 PASS — known-role goggles coherence.

Trader source / transfer:
- 211 PASS — true runtime trader inventory can roll random legal generic profession stock.
- 214 PASS at same-engine-item transfer level — trader-generated item kept its exact persistent affix moving trader -> player.
- 215 PASS — transferred trader item carried/unequipped gives no ProfessionGear bonus.
- 216 PASS — transferred item activates its stored affix when equipped.
- 219 PARTIAL — `Character::isATrader()` source detection works on true trader actors; still need naturally generated shop-stock ownership confirmation rather than only harness-injected merchant inventory.
- 237 PARTIAL — trader random legal distribution demonstrated, but broader real shop-stock distribution still pending.

Backpack control:
- 117 PASS — generic/thief backpack did not receive specialist contents-weight reduction.
- 299 PASS (weight-control half) — generic pack no specialist weight reduction.

#### Harness-only improvements already installed

Stobe test harness currently includes:
- exact GameData stringID matching,
- all inventory-section item lookup,
- `traders [radius]` runtime isATrader discovery,
- `transfer <from> <to> <item>`,
- `packput <npc> <pack> <item> [n]`,
- `packweight <npc> <pack>`,
- CONTAINER GameData support in item lookup/create paths,
- `craftfinish <npc> <item>` using nearest real CraftingBuilding::addFinishedCraftItem().

These are test-harness changes only, not ProfessionGear gameplay features.

#### Last prepared-but-not-run batch

The 20:51 archive was taken immediately before testing the new CONTAINER/craft harness batch.

Next exact live work:
1. Launch dedicated `pg-context-base` fixture.
2. Validate `craftfinish` against a real nearby CraftingBuilding:
   - if a real bench exists, test 85/89/306 craft hook/final-quality path;
   - if no real bench exists, record safe harness failure and keep Phase 3 pending.
3. Use exact CONTAINER IDs to create/test specialist packs:
   - equipped vs unequipped weight behavior,
   - matching vs unrelated cargo,
   - generic pack control already passed.
4. Continue real-trader/natural-shop-stock validation.
5. Review the live `Staff -> TOOL_FARMING` classification before accepting it; confirm name/description evidence or add a false-positive regression.

#### Timeout/process-exit finding

The run that ended before this recovery did not leave a new Kenshi crash dump. RE_Kenshi rendered normally and ProfessionGear loaded all hooks. Treat the stopped process as harness/tool-session termination unless new evidence shows otherwise.


### Resume live batch — crafting fixture probe

`pg-context-base` loaded successfully and harness health was good.

`craftfinish Shay 2309-clothes_v1.mod` returned:
- `no CraftingBuilding within 300`

Disposition:
- harness failed safely as designed,
- no ProfessionGear pass/fail assigned,
- Phase 3 / tests 85–92 and 306 remain **PENDING FOR VALID CRAFTING FIXTURE**,
- do not fabricate a craft pass without a real CraftingBuilding.

Next: continue prepared specialist-pack/container batch.
