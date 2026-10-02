# Profession Gear Progression — ACTIVE CONTEXT

**Last updated:** 2026-10-02
**Purpose:** Live handoff file. Update this file every implementation turn before finishing so another agent can continue without reconstructing state.
**Hard constraint:** DO NOT install, copy, enable, or otherwise apply this mod to the Kenshi game directory until Shay explicitly authorizes it.

## Project locations

- Design/source context: `C:\KenshiModding\PROFESSION_GEAR_PROGRESSION_MOD_CONTEXT.md`
- Canonical standalone project/repo: `C:\KenshiModding\Kenshi-Profession-Gear-Progression`
- Previous in-monorepo snapshot: `C:\KenshiModding\components\ProfessionGear` (do not treat as canonical after standalone split)
- Kenshi SDK used for compile: `C:\StobeBuild\sdk`
- VS2010 portable toolchain: `C:\StobeBuildTools`
- Current package output: `C:\KenshiModding\components\ProfessionGear\out\package\ProfessionGearProgression`
- Current game install check used previously: `D:\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression`
- The game mod directory above was checked and the ProfessionGearProgression folder was NOT present.

## Git state / history

- Repository: `C:\KenshiModding`
- Branch: `main`
- Remote: `origin https://github.com/shaylanger/Kenshi-Stobe-Custom.git`
- ProfessionGear initial implementation commit: `b0c29f1 feat: add profession gear progression mod foundation`
- That commit was pushed to origin/main.
- There are unrelated modified/untracked files in the repo. Do not stage or alter them unless directly required.
- Current unrelated tracked modification observed: `tools/stobe-reset-npc`
- Current unrelated untracked files include root context docs and several tools scripts.

## What is implemented now

### Core
- Standalone SDK-independent `ProfessionGearCore`.
- Contextual item tags for Farming, Mining/Labouring, Research, Engineering, Robotics, Medic, smithing, Cooking, work/travel boots, specialist packs, Turrets, Scout and Stealth.
- Profession stat mapping includes Labouring, Science, Engineering, Robotics, Weapon/Armour/Crossbow Smithing, Medic, Turrets, Farming, Cooking, Athletics, Swimming, Perception, Stealth, Assassination, Lockpicking and Thievery.
- Item quality -> 7 internal tiers.
- Tier-specific affix chance and magnitude range.
- 1 affix normally, optional second coherent affix at higher quality.
- Deterministic per-instance RNG keyed by item handle/base ID.
- Role-aware roll weighting.
- Player-crafted chance multiplier.
- Poor/slave suppression.
- Stackable items excluded.
- TSV serialization/parser for per-instance affix persistence.

### Game plugin
File: `src\ProfessionGearPlugin.cpp`

Hooks:
- `PlayerInterface::update`
- `CharStats::getStat(StatsEnumerated,bool)`
- `CraftingBuilding::addFinishedCraftItem(Item*)`
- `Inventory::getTotalWeight()`
- tooltip `getTooltipData1` for InventoryItemBase, Armour, ContainerItem, Crossbow and Sword.

Behavior:
- Scans loaded characters once per second from `GameWorld::getCharacterUpdateList()`.
- Discovers eligible items and gives each one a persistent roll once.
- Uses character profession stats to infer primary profession context.
- Reads slave state and suppresses special rolls for slaves/very poor NPCs.
- Effective profession stat bonus is applied only when the affixed item is equipped.
- `getStat(..., true)` (unmodified/base stat) is left untouched.
- Effective stat result is capped at 150.
- Crafted eligible item is rolled at craft completion using final item quality.
- Tooltip appends a `Profession Gear` section with stat bonuses.
- Specialist backpack weight path exists for ore, crop, construction, medical, tech and trade packs.
- Backpack specialization preserves Kenshi's original total and applies a category-specific ratio to contents.
- Master `Enabled` switch bypasses scanning/stat/tooltips/backpack changes.
- `AutoClassify` can be disabled.
- Exact rules support:
  - `exclude|base_item_string_id`
  - `tag|base_item_string_id|TAG1,TAG2`
- Rules override automatic classification.
- Persistence file: `profession_gear_affixes.tsv`
- Log file: `ProfessionGear.log`

### Build/package
- `build_portable.bat`: VS2010/Kenshi SDK DLL build.
- `run_tests.bat`: SDK-independent core test build/run.
- `package.bat`: copies built DLL/config/rules/RE_Kenshi manifest/mod.info into project-local package output only.
- Current DLL build previously completed with `BUILD OK`.
- Export check previously confirmed undecorated `startPlugin`.
- Core tests previously printed `ProfessionGearCore tests passed`.
- Package previously completed with `PACKAGE OK`.

### Documentation/tests
- `README.md`
- `TEST_PLAN.md`
- Test plan currently has 160 numbered tests spanning build/core, hook safety, per-instance identity, effective stats, crafting, NPC context, specialist packs, persistence, tooltip/UI, compatibility and stress/performance.
- Existing `tests\test_profession_gear.cpp` covers only a subset of the 160 plan and needs expansion.

## Important gaps found during resume audit

1. **Automated test coverage is incomplete.**
   The 160-case plan exists, but the SDK-independent executable currently tests only basic classification, tiers, deterministic rolls, serialization and aggregation.

2. **Runtime architecture still polls loaded characters every second.**
   Original design prefers event-driven item/equipment/inventory hooks and cached character aggregates. Current stat path scans the character inventory on every hooked stat read to sum equipped affixes. This should be hardened/cached before claiming the full runtime implementation is complete.

3. **General utility progression is incomplete.**
   Athletics/Perception/Stealth are present as stat affixes and specialist pack weight exists, but explicit generalized movement-speed/carry-efficiency/encumbrance affix types are not yet modeled independently.

4. **Specialist backpack stacking is not implemented.**
   Category-specific weight reduction exists. The design also calls for stronger specialist stacking where feasible. Need decide safe implementation without touching installed FCS.

5. **No new FCS content has been authored.**
   This is intentional so far because the user prohibited applying the mod to the game. The runtime system can augment existing/third-party items via classification/rules. A true FCS package with new tool/clothing/backpack records would require either an offline-safe FCS workflow or later authorized work in the Kenshi mod environment. Do not silently touch the game directory.

6. **Game-facing behavior is compile-validated, not live-validated.**
   No Kenshi runtime hook or save/load test has been executed because installation is explicitly prohibited.

7. **Stable item-handle persistence remains an in-game unknown.**
   Sidecar persistence uses `Item::getHandle().toString()`. It must be proven through save/load, transfer, stream-out/in and import once game testing is authorized.

8. **Job code-path coverage remains an in-game unknown.**
   Hooking `CharStats::getStat` compiles, but it must be proven that Farming/Labouring/etc. production calculations use this getter rather than raw fields in all important paths.

## Current task from Shay

Continue building the full mod without installing/applying it to Kenshi. Add/complete a full test plan using the existing testing framework. Maintain this ACTIVE_CONTEXT.md every turn with:
- what was inspected,
- what changed,
- test/build status,
- blockers/unknowns,
- exact next work.

## Work planned next

1. Inspect existing KenshiModding automated testing framework and reuse its conventions where practical.
2. Expand core representation to support utility effects beyond profession stat-percent affixes, especially carry/encumbrance/movement/logistics where they can be tested offline.
3. Replace hot-path inventory rescans with cached equipment aggregates and add event/inventory invalidation hooks where SDK symbols permit.
4. Add specialist pack stacking behavior if a safe runtime mechanism exists.
5. Expand SDK-independent automated tests substantially so the plan is backed by executable coverage.
6. Add static symbol/build validation scripts for required Kenshi hook exports.
7. Re-run all offline tests/build/package.
8. Update this context with exact results.
9. Do NOT install the package into Kenshi.

## Notes for next agent

- Preserve C++14/VS2010 compatibility.
- Follow existing STOBE build/toolchain patterns.
- Do not assume CMake is on PATH; the portable batch build is the known-good route.
- Avoid touching unrelated repo files.
- Build success does not equal in-game success.
- Do not state that live hooks, item handle persistence, job calculations or UI are proven until the mod is actually authorized for installation and tested.


---

## Turn update — 2026-10-02 continuation after timeout

### Resume audit performed
- Confirmed the prior ProfessionGear implementation and package were intact.
- Confirmed initial implementation commit `b0c29f1` exists.
- Reviewed the full original profession-gear design context again.
- Reviewed the existing STOBE automated test framework and its game-thread scenario philosophy.
- Confirmed this mod is still NOT installed/applied to Kenshi.

### Changes made this turn

1. **Created this ACTIVE_CONTEXT.md** as the mandatory live handoff file.
2. **Moved effective-stat math into the testable core** via `EffectiveStatValue(...)`.
3. **Moved specialist-pack item weight classification into the testable core** via `SpecialistPackItemWeightMultiplier(...)`.
4. **Added per-character equipped-bonus caching**:
   - cache key: character handle serial,
   - cached value: profession-stat -> total equipped affix percent,
   - `CharStats::getStat` now reads the cache instead of rescanning the whole inventory each stat query.
5. **Added inventory event refresh hooks**:
   - `Inventory::_sectionAddItemCallback`
   - `Inventory::_sectionRemoveItemCallback`
   - `Inventory::_sectionUpdateItemCallback`
   These rebuild the affected character's cached profession aggregate after the original callback.
6. The existing once-per-second loaded-character scan remains intentionally as a discovery/safety fallback for NPC initialization/stream-in and unknown equipment lifecycle cases.
7. Removed duplicated specialist-pack weight matching logic from the plugin; the core is now the single source for that behavior.
8. **Expanded offline executable coverage from a small smoke suite to 5,135 checks**, including:
   - exact quality boundaries,
   - tier range/chance monotonicity,
   - classification for every currently modeled item family,
   - override/exclusion precedence,
   - allowed-stat pools,
   - effective-stat base/unmodified/cap/floor behavior,
   - every specialist pack weight family and unrelated-item negative cases,
   - deterministic/instance-varying rolls,
   - stackable/disabled/zero-chance handling,
   - high-tier/max-affix behavior,
   - serialization validation,
   - aggregation,
   - RNG bounds/distribution sanity,
   - hash determinism,
   - profession-stat enum recognition.
9. **Added `verify_sdk_symbols.ps1` and `verify_offline.bat`**.
   It checks all current hook symbols against `C:\StobeBuild\sdk\KenshiLib.lib` and checks the built DLL export table for `startPlugin`.
10. Updated README and TEST_PLAN to describe current cache behavior and offline validation commands/results.

### Validation results this turn

- `run_tests.bat`:
  - **PASS**
  - `ProfessionGearCore tests passed: 5135 checks`
- `build_portable.bat`:
  - **PASS**
  - `BUILD OK`
- `verify_offline.bat`:
  - **PASS**
  - 13/13 checks
  - verified:
    - PlayerInterface::update
    - CharStats::getStat
    - CraftingBuilding::addFinishedCraftItem
    - Inventory::getTotalWeight
    - Inventory add/remove/update callbacks
    - base/Armour/Container/Crossbow/Sword tooltip symbols
    - DLL `startPlugin` export
- `package.bat`:
  - **PASS**
  - package remains project-local under `out\package\ProfessionGearProgression`

### Current implementation assessment

The runtime framework is now built far enough for pre-install status:
- generalized contextual classification,
- quality/tier rarity and magnitude,
- role-aware NPC rolls,
- slave/poor suppression,
- crafting integration,
- per-instance sidecar persistence,
- cached equipped profession stat effects,
- specialist category weight backpacks,
- tooltip hooks,
- explicit compatibility rules,
- offline build/symbol/core validation.

### Still intentionally unproven / blocked by "do not apply to game"

These cannot be truthfully completed without loading the plugin in Kenshi:
1. Whether item handles remain stable through real save/load, streaming, transfers and imports.
2. Whether every profession job path actually calls the hooked `CharStats::getStat` rather than reading raw struct fields.
3. Live tooltip dispatch/chaining and visual layout.
4. Live plugin-hook coexistence with STOBE and KenshiFP.
5. Live specialist backpack UI weight behavior.
6. NPC initialization timing across actual stream-in/out.
7. Long-duration performance in a real loaded world.
8. Exact game save/import policy for sidecar identity.

### Deliberate non-implementation: selective stacking
The design wants specialist backpacks to stack their intended cargo better. Kenshi exposes section-wide `setStackingBonus` / `getMaxStack` but those calls do not expose the current item category. Applying them would boost unrelated contents too, violating the specialist-pack design. No unsafe/general stacking override was added. Keep this as an in-game/FCS design item rather than pretending it is solved.

### FCS content status
No new FCS item records have been authored because doing so through Kenshi's mod environment risks crossing the user's explicit "do not apply the mod to the game yet" boundary. The native framework can operate on existing/third-party items through classification and exact rules. A later authorized phase can add new named profession tools/clothing/packs as FCS content.

### Exact next work
1. Run final repository/game-directory safety check.
2. Run `git diff --check`.
3. Stage/commit/push only `components/ProfessionGear` changes from this turn.
4. Keep all unrelated repo changes untouched.
5. Await explicit authorization before any install/live-game testing.


---

## Turn update — 2026-10-02 Git repository split discussion

### Request
Shay asked whether ProfessionGear can be moved into its own new Git repository and whether the agent can create/push it directly.

### Environment check
- Git CLI is installed: `git version 2.51.1.windows.1`.
- GitHub CLI (`gh`) is not installed/available on PATH.
- No GitHub connector/tool is currently available in this session for creating a remote repository directly.

### Recommended repository split
- Keep active source at `C:\KenshiModding\components\ProfessionGear` until the remote is created or a dedicated local checkout path is chosen.
- Best clean target is a standalone repository containing the current contents of `components\ProfessionGear` at repository root.
- Do not carry unrelated KenshiModding history/files into the new repository.
- Preserve this ACTIVE_CONTEXT.md in the standalone repo.

### Next steps once a remote exists
1. Create/init standalone local repo from current ProfessionGear directory (or copy to a dedicated path if Shay prefers).
2. Add a standalone .gitignore suitable for generated DLL/OBJ/PDB/build output.
3. Make an initial repository commit containing source/docs/config/test scripts/context.
4. Add the new GitHub remote.
5. Push main.
6. Verify remote tracking and clean status.
7. Do not install/apply the mod to Kenshi.


---

## Turn update — 2026-10-02 standalone repository creation

### Repository target
- GitHub remote supplied by Shay: `https://github.com/shaylanger/Kenshi-Profession-Gear-Progression`
- Canonical local checkout created at: `C:\KenshiModding\Kenshi-Profession-Gear-Progression`
- The previous monorepo copy at `C:\KenshiModding\components\ProfessionGear` is now a historical snapshot, not the canonical working tree.

### Repository contents prepared
- Copied all ProfessionGear source/docs/tests/config/build scripts into the standalone directory.
- Added standalone `.gitignore` excluding generated native outputs, runtime sidecar/log files, and editor/temp files.
- Copied the full original design brief into this repo as `DESIGN_CONTEXT.md` so the standalone repository is self-contained.
- Generated `obj/` and `out/` directories may exist locally for validation but are intentionally ignored by Git.

### Next actions in this turn
1. Re-run tests/build/symbol verification from the standalone checkout.
2. Initialize Git locally.
3. Add the GitHub remote above.
4. Commit source/docs/tests/config/context only.
5. Push `main`.
6. Verify clean tracking state and remote.
7. Do not install or apply the mod to Kenshi.


---

## Turn update — 2026-10-02 standalone GitHub repository created

### Standalone repository is now live
- GitHub: `https://github.com/shaylanger/Kenshi-Profession-Gear-Progression`
- Local canonical checkout: `C:\KenshiModding\Kenshi-Profession-Gear-Progression`
- Branch: `main`
- Remote: `origin`
- Initial standalone root commit: `e161da1 feat: initial profession gear progression mod`
- Push to `origin/main` succeeded.

### Repository hygiene
- Added standalone `.gitignore`.
- Generated `obj/`, `out/`, DLL/EXE/PDB/LIB/EXP/log/runtime sidecar artifacts are ignored and are not part of the source commit.
- Added `DESIGN_CONTEXT.md` so the full original design/feasibility context travels with this repo.
- Kept `ACTIVE_CONTEXT.md` in the repository for future agent handoff.
- Cleaned Markdown trailing whitespace in the copied design context after the first push.

### Validation performed from the standalone checkout before push
- `run_tests.bat`: PASS — 5,135 checks.
- `build_portable.bat`: PASS — BUILD OK.
- `verify_offline.bat`: PASS — 13/13 symbol/export checks.
- `package.bat`: PASS.
- No files were installed into Kenshi.

### Canonical development rule from this point
All future Profession Gear implementation work should be done in:
`C:\KenshiModding\Kenshi-Profession-Gear-Progression`

Do not continue feature development in:
`C:\KenshiModding\components\ProfessionGear`
unless explicitly reconciling/removing the old snapshot.


---

## Turn update — 2026-10-02 balance calibration framework

### Request
Shay asked to add tests that determine good balance values for tier bonuses instead of relying on placeholder intuition.

### Changes made
1. Added `BALANCE_TEST_PLAN.md`.
2. Extended the master `TEST_PLAN.md` from 160 to **210 tests**.
3. Added tests 161–210 specifically for balance calibration:
   - Farming baseline/response curves and full-set progression.
   - Labouring/mining response curves and full-set progression.
   - Science, Engineering, Robotics, Cooking, smithing, Medic and Turret calibration.
   - Athletics, Swimming, Stealth, Lockpicking, Assassination, Thievery and Perception calibration.
   - Cross-profession fitting, skill-vs-gear sanity checks, full-set caps and final v1 range freeze.
4. Defined controlled base-skill bands: 10, 25, 50, 75, 90.
5. Defined controlled effective-bonus sweep: +2, +4, +6, +8, +10, +15, +20, +25, +35, +50%.
6. Defined repeat policy: minimum 5 runs per point; 10 if variance exceeds 3%.
7. Defined real throughput metric:
   `gain% = (baseline_time / modified_time - 1) * 100`
8. Added initial target gameplay bands to validate rather than assume:
   - one Shoddy piece: 2–5% real throughput,
   - full Shoddy set: 10–15%,
   - full mid-tier set: 15–25%,
   - full high/specialist set: 25–35%,
   - full top-tier set: 35–50%.
9. Explicitly requires profession-specific tier ranges if real response curves differ materially. We will not force a universal percent table if Farming, Labouring, Science, etc. react differently.
10. Added `tools/analyze_balance.py`:
    - reads live measurement CSV,
    - groups by profession/base skill/effective bonus,
    - computes median/mean/stddev throughput,
    - compares each point with the 0% baseline,
    - reports real throughput gain,
    - proposes candidate effective-skill bonus bands at base skill 50,
    - reports an insufficient sweep rather than pretending a target was reached.
11. Added:
    - `tests/fixtures/balance_results_template.csv`
    - `tests/fixtures/balance_sample.csv`
12. Tested analyzer against synthetic Farming and Labouring fixture data. It correctly produced response curves and flagged top-tier target bands as insufficient when the synthetic sweep did not reach them.
13. Updated README to state that current tier values are provisional until calibration is run.

### Important limitation
No real balance result has been claimed yet. These are the tests/harness needed to derive it. Actual job-throughput data still requires loading the plugin into Kenshi, which remains prohibited until Shay explicitly authorizes installation/testing.

### Next work
- Keep current placeholder affix values unchanged until real calibration data exists.
- When live testing is authorized, add ProfessionGear scenario commands/test bridge to set controlled skill/affix/loadout state and automatically emit the CSV format above.
- Run Farming and Labouring first; fit their curves before expanding to every profession.


---

## Turn update — 2026-10-02 vanilla gear classification correction + 3-affix top tier

### Request
Shay asked whether the existing classifier was based on real Kenshi gear, asked for a review of the full vanilla gear catalogue, requested top tiers to support up to 3 stats, and asked how mod-added gear can still work if exact IDs are preferred.

### Findings
- Earlier examples such as lab coats, chef aprons, research tools and hoes were partly conceptual targets, not faithful vanilla equipment families.
- Reviewed the real vanilla equipment families from the installed game/wiki references:
  - headgear,
  - shirts,
  - body armour,
  - legwear,
  - footwear,
  - backpacks,
  - melee weapon classes,
  - crossbows,
  - robot limbs,
  - lanterns.
- Added `VANILLA_GEAR_CLASSIFICATION.md` with the real catalogue and revised policy.
- Important real examples:
  - Straw Hat exists and its vanilla description explicitly references peasant farmers/desert scouts.
  - Square Goggles and Ashlander Stormgoggles exist and provide native Perception, but vanilla does NOT call them research gear.
  - Wooden Sandals already improve Athletics/combat speed.
  - Scout Legs, Stealth Legs, Thief's Arm, Steady Arm and Industrial Lifter Arms have strong native semantic stat signatures.
  - Generic armour/weapons should not automatically receive profession stats.

### Revised classifier strategy
Not ID-only and not keyword-only.

Priority/evidence:
1. explicit exact-ID rule,
2. native object type/slot,
3. existing native mechanical fingerprint,
4. description / linked GameData semantics where available,
5. semantic name tokens as fallback,
6. NPC/source context only for roll weighting among already-legal stats.

Weak or contradictory evidence => no profession classification.

This keeps third-party mod support because unknown IDs can still classify from slot/type + native stats + descriptions + semantic tokens. Exact IDs remain authoritative overrides/corrections, not the only route.

### Affix count change
Implemented requested tier ceilings:
- tiers 0–2: max 1 affix,
- tiers 3–4: up to 2 affixes,
- tiers 5–6: up to 3 affixes.

`MaxAffixes` default changed from 2 to 3.
Extra affixes remain probabilistic and must come from the coherent legal stat pool. A one-stat item still cannot magically get three unrelated stats.

### Validation
- `run_tests.bat`: PASS — 5,140 checks.
- `build_portable.bat`: PASS — BUILD OK.
- `verify_offline.bat`: PASS — 13/13 symbol/export checks.
- Mod remains uninstalled.

### Next work
1. Replace the current simple name-heavy runtime classifier with the scored hybrid evidence model documented in `VANILLA_GEAR_CLASSIFICATION.md`.
2. Add explicit native mechanical fingerprint fields to `ItemDescriptor` where SDK access is safe.
3. Add curated vanilla mappings only where profession semantics are defensible.
4. Add classifier tests against a fixture catalogue of real vanilla items plus synthetic third-party items with unusual names.
5. Keep exact rules as the final authority.
