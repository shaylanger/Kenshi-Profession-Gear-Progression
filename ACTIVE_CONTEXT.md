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


---

## Turn update — 2026-10-02 classifier correction for plain modded items + installed mod audit

### User correction / requirements
Shay clarified:
- third-party mods will normally add plain Kenshi items with no ProfessionGear metadata or native profession stats,
- item **name** must therefore be a primary classifier signal,
- examples include Green Hat, Running Shoes, Worker Rags, ordinary new weapons, etc.,
- same base item may plausibly support multiple professions (e.g. generic rags: Farming or Labouring depending on wearer),
- goggles may plausibly support several categories,
- very high/unique named weapons should not be modified,
- weapons have many more grades than armour and must be handled separately,
- installed mods should be used as a real classifier corpus.

### Installed mod audit performed
Visible deployed mods include:
- GenMod
- Universal Wasteland Expansion
- Dust
- Unofficial Patches for Kenshi
- Fixing Clipping Issues
- Compressed Textures Project
- detail textures
- plus UI/animation/runtime mods.

Asset scan:
- UWE: 4673 files, ~1422 filenames/paths matching gear/weapon/armour patterns.
- GenMod: 574 files, ~47 gear-looking files.
- UWE deployed item assets include extensive armour/headwear/boots/bags/weapons and a real Pickaxe mesh.
- GenMod deployed assets include multiple backpack meshes and Pickaxe industry assets.
- UWE's Vortex source .mod is accessible (~41 MB).
- GenMod's deployed .mod symlink points at a stale/missing Vortex source path, but its deployed assets remain readable.

Added `INSTALLED_MOD_GEAR_AUDIT.md` documenting these observations and future corpus-testing plan.

### Classifier implementation changes
1. `ItemDescriptor` now includes:
   - description,
   - weapon level,
   - weapon/armour/robot-limb/container flags,
   - legendary protection flag.
2. Plugin `Describe()` now:
   - reads normal item description from GameData where present,
   - uses real runtime class (Weapon/Armour/RobotLimb/Container),
   - uses `Gear::getLevel01()` for gear quality,
   - captures `level_0_100` for weapons,
   - marks level-100 / Cross / Meitou / explicitly legendary weapons protected.
3. Normal combat weapons are hard-rejected from profession classification unless name/description clearly identifies an actual profession tool such as Pickaxe/Hoe/etc.
4. Added `WORKWEAR_GENERIC`:
   - plain hats / Worker Rags / basic work clothing can legally support Farming, Labouring, Engineering or Cooking.
   - first affix is resolved to the wearer's primary profession when it is inside that pool.
   - therefore the exact same Worker Rags base item can become Farming on a farmer and Labouring on a labourer.
5. Added `GOGGLES_GENERIC`:
   - plain goggles/glasses/visors can support Perception, Science, Engineering, Robotics or Turrets.
   - wearer/source context resolves the first affix.
6. Added Running Shoes / sneaker semantic detection to Athletics/travel even when there is no native Athletics stat.
7. Updated documentation so **name + description are first-class semantic evidence**. Native stats are optional corroboration only and are never required for third-party support.

### Weapon grade handling
Confirmed vanilla weapon ladder is separate from armour quality.
Core now recognizes the 15 weapon grades/model levels:
- Rusted Junk
- Rusting Blade
- Mid-Grade Salvage
- Old Refitted Blade
- Refitted Blade
- Catun No.1
- Catun No.2
- Catun No.3
- Mk I
- Mk II
- Mk III
- Edge Type 1
- Edge Type 2
- Edge Type 3
- Meitou

Implementation:
- added `WeaponGradeRank(level_0_100)`,
- added `ProgressionTier(ItemDescriptor)`,
- weapon grades map into the internal affix-balance bands instead of being treated like armour quality,
- Meitou/level-100 is protected,
- Edge-grade (>=70) weapon instances on unique named NPCs are protected by default,
- ordinary combat weapons are normally outside the profession-affix system anyway.

### Tests added
New offline cases cover:
- Green Hat => generic workwear,
- Worker Rags => contextual Farming vs Labouring on different wearers,
- plain goggles => multi-context pool,
- researcher goggles => Science first affix,
- Running Shoes without native Athletics => travel/Athletics,
- Farmer's Sword => combat gate rejection,
- weapon-slot Pickaxe => allowed Labouring tool,
- legendary item => rejected,
- exact weapon grade rank mappings,
- internal progression tier mapping for weapon grades.

### Validation this turn
- `run_tests.bat`: PASS — **5,164 checks**.
- `build_portable.bat`: PASS — BUILD OK.
- Mod remains uninstalled.

### Next work
1. Build a reusable classifier fixture corpus from real vanilla + UWE + GenMod names/assets.
2. Improve plain clothing/headwear type gates further so e.g. generic heavy/ceremonial hats do not enter workwear pools too easily.
3. Add description-driven synthetic cases for strangely named mod items.
4. Add curated protection rules for any unique/legendary mod items discovered in the installed corpus.
5. Keep real live item enumeration as a later authorized in-game/FCS step; do not install ProfessionGear yet.


---

## Turn update — 2026-10-02 equip-only clarification + profession weapon semantics + category coverage

### Equip behavior clarified
- Profession affix records can exist on an item while it is merely carried.
- **No stat bonus is active unless `item->isEquipped` is true.**
- This is already enforced by the runtime equipped-bonus cache.
- Test 48 now explicitly states that carrying an eligible item grants zero bonus.
- UWE's Pickaxe appears in `items\weapons\mesh\pickaxe.mesh`, so it is likely an equipable weapon-class item.
- GenMod's `Pickaxe_Deco` is under industry/building assets and appears to be a prop, not equipable character gear. World/building props are outside ProfessionGear character stat bonuses.

### Weapon semantic correction
Shay explicitly wants roleplay semantics to beat the previous combat-weapon rejection when the name clearly implies a profession.

Changed classifier:
- `Farmer's Sword` can classify Farming.
- Pitchfork weapon-class items classify Farming.
- Pickaxe weapon-class items classify Labouring.
- Chef/cooking-named weapons can classify Cooking.
- Engineer/builder/mechanic-named weapons can classify Engineering.
- Medic/doctor/surgeon-named weapons can classify Medic.
- Research/science-named weapons can classify Science/Research.
- Smithing/forge-named weapons can classify smithing.
- Ordinary weapons with no profession semantics remain excluded.
- Legendary/unique protections still win.

### Generic-context fallback expanded
`WORKWEAR_GENERIC` now supports:
- Farming
- Labouring
- Engineering
- Cooking
- Medic
- Science
- Robotics
- Weapon Smithing
- Armour Smithing
- Crossbow Smithing

But generic workwear/goggles now **require a matching role context**. A contextless generic rag/hat does not randomly choose a profession. This prevents random loot from becoming nonsensical while allowing:
- farmer rags -> Farming,
- labourer rags -> Labouring,
- field medic rags -> Medic,
- cook rags -> Cooking,
- engineer workwear -> Engineering,
- researcher workwear/goggles -> Science,
etc.

### Installed-content evidence
UWE database string scan found:
- equipable-looking Plague Doctor mask assets and doctor/field-medic contexts,
- Pickaxe item and weapon mesh,
- Sickle weapon mesh,
- extensive Farming/worker/mining contexts,
- Engineer NPC/shop/construction contexts,
- abundant Tech Hunter/research contexts,
- cook NPC/cooking contexts.

No clearly dedicated wearable Engineer/Research/Cooking sets were identified from the string scan. Therefore those categories currently rely more on contextual generic clothing/goggles unless later record-level audit finds dedicated items.

Added a category availability matrix to `INSTALLED_MOD_GEAR_AUDIT.md`.

### Current assessment
ProfessionGear itself does not spawn new gear; it augments existing instances.
- Strong natural coverage: Farming, Labouring, Turrets/Perception, Athletics/travel, Stealth.
- Moderate but representable coverage: Medic, Engineering, Science, Robotics, Cooking, Smithing.
- The live test phase must measure actual category frequency. If a category is too rare, use curated mappings or optional FCS profession items rather than assigning stats to unrelated equipment.

### Validation
- `run_tests.bat`: PASS — 5,169 checks.
- `build_portable.bat`: PASS — BUILD OK.
- Mod remains uninstalled.


---

## Turn update — 2026-10-02 trader stock profession gear + mandatory live-test recording

### User requirement
Shay wants browsing shops to be a real way to discover profession gear. Generic items sold by traders should have a chance to roll any coherent stat in the item's allowed context pool even if no wearer profession exists yet.

Shay also explicitly required that anything described as needing in-game validation must be recorded as an actual numbered test, not left as an informal note.

### Trader-stock behavior implemented
- Added `RoleProfile::traderSource`.
- Runtime `RoleFor(Character*)` now uses Kenshi SDK `Character::isATrader()`.
- When an item belongs to a trader and is **not equipped**, its shop-stock roll ignores the shopkeeper's unrelated strongest skill and treats the item as trader stock.
- Contextual generic shop items may therefore roll randomly from their legal profession pool.
- Example generic workwear at a shop may become Farming, Labouring, Engineering, Cooking, Medic, Science, Robotics, or Smithing gear, depending on the item pool.
- Generic goggles in shop stock may roll Perception, Science, Engineering, Robotics, or Turrets.
- Loose/contextless generic items that are not trader stock and have no wearer profession still get no profession roll.
- Equipped gear on the trader still uses the trader character's own role context rather than shop-stock randomization.
- Buying an item should preserve the instance affix; active stat bonus still requires the purchased item to be equipped.

### Important live uncertainty now explicitly tested
The SDK exposes `Character::isATrader()`, but we have not yet proven whether every real Kenshi vendor's sellable stock is physically stored in the trader Character inventory versus another shop/container structure. This is now test 219. If it is not, add the appropriate shop-container hook rather than silently missing vendor stock.

### Test plan expansion
Added Section M, tests **211–240**, covering:
- trader-stock generation,
- generic shop workwear/goggles,
- contextless loose loot negative case,
- purchase persistence,
- carried-vs-equipped activation,
- shop restocking,
- trader personally equipped gear,
- actual vendor stock ownership structure,
- per-profession vendor availability for Farming/Labouring/Medic/Engineering/Science/Robotics/Cooking/Smithing,
- NPC/loot profession distribution,
- equip-only UWE tool weapons,
- non-equippable GenMod decorative tools,
- profession-named weapons,
- ordinary combat-weapon negatives,
- Meitou/Cross protection,
- unique Edge-grade protection,
- generic context resolution,
- shop-stock distribution quality,
- performance on large mod load,
- save/load and import behavior.

This makes the master numbered plan extend through **240**.

### Release rule
If live tests show a profession is technically supported but too rare in vendors/NPCs/loot, fix distribution using curated mappings or optional FCS profession items. Do not make unrelated equipment roll nonsensical stats merely to increase frequency.


---

## Turn update — 2026-10-02 exploration/world-loot profession gear

### User requirement
Shay wants exploration itself to be a viable source of profession gear. A generic eligible item found in a ruin/chest with no NPC/trader context should still have a chance to become profession gear, e.g. a Rag Shirt with Cooking.

### Behavior implemented
- Added `RoleProfile::worldLootSource`.
- Added config `WorldLootMultiplier=0.50`.
- Contextless exploration gear can now roll randomly from its **legal item pool**.
- The world-loot multiplier is applied to the normal tier affix chance, so exploration rolls are rarer than ordinary context-matched NPC gear by default.
- Example generic workwear found in a ruin may become Farming, Labouring, Engineering, Cooking, Medic, Science, Robotics, or Smithing gear.
- It still cannot roll a stat outside its allowed semantic pool.
- World loot remains equip-only for active bonuses.

### Player-profession bias fix
The current plugin primarily discovers items through character inventories.
Therefore an unseen chest item may first become visible to ProfessionGear when the player picks it up.
Without a special rule, that would incorrectly generate the item from the player's profession.

Implemented:
- when a previously unseen, non-crafted eligible item is first observed on a player character, it is treated as `worldLootSource`,
- the player's primary profession is cleared for that generation event,
- the roll is therefore random within the item's legal pool and independent of who picked it up,
- once the record exists, normal instance persistence prevents rerolls.

Existing NPC/trader items should already have records before transfer if their source inventory was scanned. If vendor stock is not exposed that way, tests 219/214 will reveal it and the fallback behavior remains coherent rather than player-biased.

### Tests added/changed
- Test 213 changed: contextless world/container loot may roll at reduced world-loot chance.
- Added Section N, tests **241–250**:
  - ruin chest pickup classified as world loot,
  - generic Rag Shirt/goggles legal-pool enforcement,
  - WorldLootMultiplier off/on statistical behavior,
  - no reroll after pickup/drop,
  - independence from picker profession,
  - unopened-chest discovery timing,
  - pre-existing player gear source handling,
  - real ruin/chest profession distribution.

Master test plan now extends through **250**.

### Important live question is explicitly tested
We do not yet know whether unopened chest contents are instantiated/exposed to the plugin before pickup.
This is now test 248. If exposed, affixes should exist before pickup. If not, pickup-time generation is acceptable as long as it is world-loot sourced, stable, and independent of the player profession.


---

## Turn update — 2026-10-02 final pre-build design decisions

### User decisions
1. Protect explicitly unique/special named items from ProfessionGear augmentation.
2. Include broader utility effects wherever they make sense, especially where Kenshi already has native mechanics for them.
3. Add profession-themed item generation/content as a formal mod goal using existing visual assets and new records/names.
4. Keep specialist-pack weight specialization as the easier v1 path; selective category stacking remains a later goal if it can be done cleanly.
5. New profession-specific FCS/content items are approved as a later phase rather than a v1 blocker.

### Unique item protection implemented now
- Runtime `Describe()` now marks **any `Item::isUnique` instance** as protected.
- Existing Meitou/Cross/level-100/legendary-name protection remains.
- This applies to weapons, armour, clothing, backpacks, etc.
- Added an offline regression covering an explicitly unique non-weapon item.

### New roadmap created
Added `ROADMAP.md` with approved goals:

#### Broad utility progression
Use native/effective Kenshi mechanics wherever sensible:
- Athletics / movement,
- Swimming,
- Perception,
- Stealth,
- Assassination,
- Lockpicking / Thievery,
- encumbrance handling,
- carry/hauling efficiency,
- specialist pack weight behavior,
- other native equipment modifiers where semantically appropriate.

#### Profession-themed item generation/content
Later optional FCS/content layer using existing Kenshi assets with new profession-themed records/names, including examples:
- Farmer's Sword / Sickle / Pitchfork,
- Miner's Pickaxe / Backpack,
- Traveler's Sandals,
- Engineer's Goggles / Workwear,
- Field Medic Coat / Mask / Pack,
- Researcher's Goggles / Coat / Satchel,
- Cook's Apron / Knife,
- Smith's Gloves / Apron / Hammer,
- specialist Ore/Crop/Construction/Medical/Tech packs.

Goal: make profession gear easier and more fun to discover without needing custom art.

#### Distribution
Generated profession variants should later participate in normal Kenshi systems:
- vendors,
- ruins/chests,
- profession/faction NPCs,
- crafting,
- blueprints/research.

The runtime per-instance affix system remains separate from the FCS base records.

#### Specialist stacking
- Weight specialization remains v1.
- Selective category stacking is a roadmap goal.
- Do not implement section-wide stacking if it boosts unrelated cargo.

### Tests recorded
Added Section O, tests **251–272**, covering:
- unique non-weapon/weapon protection,
- normal high-quality non-unique eligibility,
- movement/carry/swimming/native utility coexistence,
- generated Farmer/Mining/Travel/Engineer/Medic/Research/Cooking/Smithing item records,
- vendor/loot/NPC/crafting distribution,
- runtime-affix integration for generated items,
- specialist stacking feasibility/fallback.

Master test plan now extends through **272**.

### Current design status
No major product/design question remains blocking the pre-install implementation.
Remaining uncertainties are technical/runtime questions and are represented by numbered tests rather than open design questions.


---

## Turn update — 2026-10-02 final pre-install implementation pass

### User instruction
Shay asked to continue implementation until ProfessionGear is finished enough to install and begin controlled in-game testing, adding tests whenever a new behavior needs coverage. The existing hard constraint remained in force: **do not install/enable/apply the mod in Kenshi until explicit authorization**.

### Pretest version
- Package/runtime version: **0.9.0-pretest**
- Canonical package:
  `C:\KenshiModding\Kenshi-Profession-Gear-Progression\out\package\ProfessionGearProgression`
- The mod remains absent from both known Kenshi install paths.

### Runtime hardening completed

1. **Real stackability detection**
   - Replaced the old quantity-only heuristic with Kenshi's real `InventoryItemBase::isStackable(InventorySection*)` path where a parent inventory/section exists.
   - Quantity-one stack-capable items are now excluded correctly.
   - Quantity remains a safe fallback if no section is available.

2. **Item-handle reuse defense**
   - Existing persisted records are only reused when the current item base ID still matches.
   - A reused handle pointing to a different base item invalidates the stale record and generates safely from the new item.
   - Unique/stackable/explicitly excluded items suppress and remove stale persisted rows instead of inheriting old affixes.

3. **Character-cache identity hardening**
   - Equipped-bonus cache now keys by the full Kenshi handle string rather than serial alone.

4. **Inventory callback reentrancy guard**
   - Add/remove/update callback refreshes cannot recursively trigger unbounded character rescans.

5. **Tooltip deduplication/protection**
   - Base/derived tooltip hook chains only append one `Profession Gear` section.
   - Unique/stackable/excluded items never display stale profession rows.
   - Current base ID must match the persisted record.

6. **Atomic sidecar replacement**
   - Sidecar writes now use `MoveFileEx(..., MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`.
   - On replacement failure the dirty state remains set so a later save can retry.

7. **Config normalization**
   - Negative chance multipliers clamp to 0.
   - `MaxAffixes` clamps to 1–3.
   - Normalization moved into SDK-independent core and is tested.

8. **Double-start protection**
   - `startPlugin()` is guarded so hooks initialize only once.

9. **Startup diagnostics**
   - Startup log identifies version and normalized config values plus override/exclusion counts.

10. **Verbose generation diagnostics**
    - New `VerboseLogging=false` config.
    - When enabled, each newly generated eligible record logs source, instance/base identity, item name, equipped state, tier, semantic tags and resulting affixes.
    - Intended for classifier/vendor/loot test runs.

### Utility progression expanded

Added tags and coherent stat pools:
- `ASSASSIN_GEAR` -> Stealth + Assassination
- `THIEF_GEAR` -> Stealth + Lockpicking + Thievery
- `SWIM_GEAR` -> Swimming
- `PACK_HAULING` -> Athletics + all-cargo weight support
- `PACK_GENERIC` -> Athletics only

Classifier additions include:
- Assassin/Ninja/Thief/Burglar semantic gear
- Swimming/diving/wetsuit/flipper gear
- Traveler/Traveller/Wanderer/Scout/Ranger gear
- hauling/cargo/load-bearing packs
- vanilla Wooden Sandals / Drifter's Boots travel semantics
- generic backpacks/bags/baskets get utility progression without specialist cargo reduction
- strongly themed utility-role weapons such as Assassin's Blade, Thief's Dagger and Scout Sword are allowed, matching the already-approved Farmer's Sword rule.

### Classifier safety improvements

- Added word-boundary semantic matching for risky short terms.
- `hoe` no longer matches `shoes`.
- `rag` no longer matches `dragon`.
- `miner` no longer matches `mineral`.
- `visor` uses word semantics in generic/research/turret paths.
- `mechanic` matching no longer causes `Mechanical Blade` to become Engineering gear.
- Strong profession/utility semantics suppress generic workwear/goggle fallback.
- Plural/common naming forms now supported: Farmers, Miners, Assassins, Thieves, Ninjas, Scouts, Rangers, Travelers/Travellers, Wanderers, Burglars.

### Profession-named pack specialization

Normal mod/FCS naming now automatically creates sensible specialist classification:
- Miner's / Miners Backpack -> Ore pack
- Farmer's / Farmers Backpack -> Crop pack
- Field Medic / Doctor Backpack -> Medical pack
- Engineer / Builder / Construction Pack -> Construction pack
- Research / Science / Robotics / Tech Satchel/Pack -> Tech pack

These specialist classifications suppress generic pack fallback.

### Backpack utility/equip behavior

- `PACK_HAULING` applies an all-cargo contents multiplier of 0.75 in the current provisional design.
- Existing specialist multipliers remain:
  - Ore 0.25 matching cargo
  - Crop 0.30
  - Construction 0.35
  - Medical 0.35
  - Tech 0.35
  - Trade 0.55 for trade goods
- Generic packs do not receive any extra contents-weight reduction.
- Extra ProfessionGear backpack weight effects now only activate when the backpack is actually equipped.
- Selective category stacking remains intentionally deferred to the roadmap because the available runtime API is section-wide and could incorrectly affect unrelated contents.

### Unique gear

Approved policy is now enforced:
- any `Item::isUnique` item instance is protected,
- Meitou/Cross/level-100/legendary markers remain protected,
- stale sidecar rows cannot bypass this protection.

### Packaging/install readiness

Added a real Kenshi launcher-recognizable package structure:
- `ProfessionGearProgression.dll`
- `ProfessionGearProgression.mod` (minimal 46-byte FCS container stub consistent with installed RE_Kenshi mods)
- `RE_Kenshi.json`
- `mod.info`
- `ProfessionGear.ini`
- `ProfessionGear.rules`
- `FIRST_LIVE_TEST.md`

Added:
- `verify_package.ps1`
  - checks required package files,
  - checks .mod is non-empty,
  - rejects leaked runtime log/sidecar state.
- `install_test_build.ps1`
  - refuses to do anything unless explicitly passed `-Install`,
  - backs up an existing ProfessionGearProgression install before replacement,
  - copies the verified package only,
  - does not alter launcher load order.
- `verify_not_installed.ps1`
  - asserts the mod is absent from known D: and C: Kenshi mod roots.
- `verify_ready.bat`
  - one-command final pre-install gate:
    1. core tests,
    2. DLL build,
    3. SDK/export verification,
    4. package creation/verification,
    5. game-directory no-install safety check.
- `FIRST_LIVE_TEST.md`
  - ordered smoke/identity/job-path/distribution/utility/coexistence/balance plan.
- `tests\fixtures\ProfessionGear.forced-test.ini`
  - forced-roll + verbose logging config for short controlled smoke tests only.

The installer refusal behavior was tested without `-Install` and correctly refused to touch the game.

### SDK/static verification expanded

`verify_sdk_symbols.ps1` now also verifies the exact SDK symbols for:
- `InventoryItemBase::isStackable(InventorySection*)`
- `Inventory::getSection(std::string const&)`
- `Character::isATrader()`
- `Character::isPlayerCharacter()`

Total final offline symbol/export checks: **17/17 PASS**.

### Test plan expansion

Master `TEST_PLAN.md` now extends through **305**.

New sections include:
- 273–290: pre-install runtime hardening/package/install safety
- 291–300: equip-only specialist utilities and semantic-boundary regressions
- 301–305: profession-named backpack specialization

SDK-independent executable coverage now passes **5,213 checks**.

### Final readiness gate

Final `verify_ready.bat` result:

- `run_tests.bat`: **PASS — 5,213 checks**
- `build_portable.bat`: **PASS — BUILD OK**
- `verify_offline.bat`: **PASS — 17/17**
- `package.bat`: **PASS**
- `verify_package.ps1`: **PASS — 6 required runtime files**
- known D: install path: **ABSENT**
- known C: install path: **ABSENT**
- final gate output: **READY FOR CONTROLLED KENSHI INSTALL/TEST**

### Deliberately not implemented before first live test

These are approved later-roadmap items, not blockers for the runtime test build:
- new profession-themed FCS content records/distribution (Farmer's Sword, Engineer gear, Field Medic sets, etc.),
- selective specialist-pack stacking,
- any direct mutation of native armour movement/combat multipliers beyond the existing safe effective-stat/weight paths.

### Remaining unknowns are live-test questions, not pre-install implementation gaps

They are all represented by numbered tests:
- hook ABI/chaining under live Kenshi,
- real item-handle stability through save/load/stream/import,
- whether all profession jobs consume the hooked effective-stat path,
- actual trader stock ownership structure,
- unopened chest item visibility timing,
- tooltip visual layout,
- live specialist backpack UI/weight behavior,
- NPC spawn/stream timing,
- long-world performance,
- STOBE/KenshiFP/full-mod coexistence,
- final balance ranges and distribution probabilities.

### Status

**The standalone runtime mod is ready for the first controlled install/live-test phase.**
Do not call it release-ready until the live suites and balance calibration pass.
