# PG in-game run order (for the coordinator)

All launches below ran and their rows passed (state 2026-10-06: `INGAME_STATUS.md`); kept as the regression run recipe.
Build hashes named below are the ones each step first needed; any later build works.

## Gate: standing step before any balance matrix (Shay 2026-10-04)

1. `python3 tools/balance_driver.py gate` (WSL) writes `tests/ingame/full-base/gate/pg-gate-*.txt` (same generator,
   fixture and method as the matrix; 4 points g1 s25 none / g2 s25 own50 / g3 s90 none / g4 s25 none).
2. Run every gate file in one launch (4080: `C:\KenshiTestRuns\pgbal-4080un4080-m27.sh kah-fullbase <files>`, list
   `gate-m29.list`), Forced + InGameTest, fresh `kah-fullbase` copy.
3. `python3 tools/balance_driver.py csv <out>` per file, then `gatecheck <csv...>`: one GATE line per profession
   (PASS / FAIL setup / measurement / flat / gear). Every point must show ready=ok, game clock advancing, equal mod=
   condition factor (and wounds=1), pg_bonus native values.
4. Classify every FAIL (setup / measurement / product / unresolved balance), fix, rerun only the failing gates.
   Only a profession whose gate passes goes to its matrix file (`pg-<n>-balance-*.txt`).
- Worker is Beaks since 0da79eb (Avarek pinned). The wounds factor 0.25 first blamed on a crippled Avarek was the harness
  protect/health bug, fixed in harness 6aa5685 (needs that harness or newer); see INGAME_STATUS.md.
- Pending after the gates: pg-89 (perception) full matrix, pg-55 (turret; d6bb99b pins the dummies), pg-51 (57e0381:
  Sickle accepted in the bench output or hauled).


Build: `out\ProfessionGearProgression.dll` **D7A60E49** (7D80DBB3 + test-only `pg_force_critical`, row 89) (`install-dll.ps1 ProfessionGear`, Kenshi closed).
All config switches are done with Kenshi closed:
`powershell -File C:\KenshiModding\Kenshi-Profession-Gear-Progression\set_test_mode.ps1 ...`
(it refuses while Kenshi runs). Logs to keep before every relaunch: `ProfessionGear.log` and
`profession_gear_affixes.tsv` in `D:\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression\`
(the log is recreated at launch). Run each file with `stobe-auto run <file> --csv <out.csv>`.

## Launch 1 — main (Forced + test rules)

Before: `set_test_mode.ps1 -Mode Forced -Rules InGameTest` (sidecar kept).

| Order | Fixture | File | Reset |
|---|---|---|---|
| 1 | auto-home | `auto-home/pg-01-startup-stability.txt` | fresh |
| 2 | auto-home | `auto-home/pg-02-equip-stats.txt` (ends with `save kah-pg-stale`, needed by launch 2) | fresh |
| 3 | auto-home | `auto-home/pg-03-identity.txt` | fresh |
| 4 | auto-home | `auto-home/pg-04-backpacks.txt` | fresh |
| 5 | auto-home | `auto-home/pg-05-npc-context.txt` | fresh |
| 6 | auto-home | `auto-home/pg-06-persistence.txt` | fresh |
| 7 | auto-home | `auto-home/pg-07-worldloot.txt` | fresh |
| 8 | auto-home | `auto-home/pg-08-job-path.txt` (~2.5 game hours at 10x) | fresh |
| 8b | auto-home | `auto-home/pg-12-job-throughput.txt` (row 177 v5: 15 min warm-up + 3 x 30 game min at 10x; needs harness F946C881; expect output_progress B ~ 1.5 x A, C ~ A) | fresh |
| 8c | auto-home | `auto-home/pg-13-farm-operate.txt` (row 331 Farming half: wheat farm A/B/C x 30 game min at 10x; scaled_amount/amount 1.0 / ~1.5 / 1.0) | fresh |
| 8d | auto-home | `auto-home/pg-14-labouring-curve.txt` (rows 178-183: 9 x 30 game min at 10x, ~30 real min; numbers only, see its verify line) | fresh |
| 8e | auto-home | `auto-home/pg-15-crowd-frametime.txt` (rows 151-152: ~5 real min at speed 1, 300 spawned Drifters; run again in launch 3) | fresh |
| 9 | auto-home | `auto-home/pg-09-soak.txt` (row 156: 30 real minutes at 50x = ~25 game hours, Shay/Malzin protected; last) | fresh |
| 10 | Crafting base | `crafting-base/pg-10-craftfinish.txt` | fresh |
| 11 | Crafting base | `crafting-base/pg-11-real-craft.txt` | fresh |
| 12 | Trader | `trader/pg-21-shop-stock.txt` (Habul's shop) | fresh |
| 13 | Trader | `trader/pg-20-shop-stock.txt` (diagnostic only, optional) | fresh |

Every file in this launch needs a fresh fixture (they change Malzin's/Shay's gear and stats).
Saves made: `kah-pg-stale`, `kah-pg-npc`, `kah-pg-persist-a/b`, `kah-pg-craft`, `kah-pg-midcraft`,
`kah-pg-trader` (keep `kah-pg-stale` until launch 2 is done; delete the rest after the run).

## Launch 2 — AutoClassify off

Before: `set_test_mode.ps1 -Mode AutoClassifyOff` (rules stay InGameTest, sidecar from launch 1 kept).

| Order | Fixture | File | Reset |
|---|---|---|---|
| 1 | auto-home (loads `kah-pg-stale` itself) | `config/pg-30-autoclassify-off.txt` | fresh |

## Launch 3 — disabled + sidecar startup fixture

Before: `set_test_mode.ps1 -Mode Disabled -Sidecar StartupTest` (launch 1's sidecar is set aside).

| Order | Fixture | File | Reset |
|---|---|---|---|
| 1 | auto-home | `config/pg-31-disabled-sidecar.txt` | fresh |
| 2 | auto-home | `auto-home/pg-15-crowd-frametime.txt` (PG disabled: the comparison run for rows 151-152) | fresh |

Check `ProfessionGear.log` for `loaded affixes=3 legacyIgnored=1` and no `roll source=` line.

## Launch 4 — normal chances (distribution)

Before: `set_test_mode.ps1 -Mode NormalVerbose -Rules Normal -Sidecar Empty`.

| Order | Fixture | File | Reset |
|---|---|---|---|
| 1 | Trader | `config/pg-40-normal-distribution.txt` | fresh |
| 2 | auto-home | `config/pg-41-normal-worldloot.txt` | fresh |

After launch 4: `set_test_mode.ps1 -Sidecar Restore` plus either `-Mode Forced -Rules InGameTest`
(more testing) or `-Mode Normal -Rules Normal` (back to normal play).

Launches 2-4 are short (one or two files each). If launches are expensive, launch 1 alone covers
95 of the 106 PENDING-auto rows.

## Regression after harness F946C881 (character handles are now `#serial/index`)

Same launch as above, fresh fixture each: pg-01...08, pg-10, pg-11, pg-21 (pg-12 first if time is short).
The files that capture handles (pg-01 `chars`, pg-05 and pg-21 `spawn`, pg-30 `spawn`) already capture
`#\d+(?:/\d+)?`, so they pass the exact `#serial/index` form on.

## New fixtures (2026-10-03, made on the 4080 without our plugins; squads are NOT Shay/Malzin)

Copy each master to a `kah-*` copy (`relaunch.ps1 -Fixture <name> -Copy <kah-name>`); don't save during the smoke
load. The first load rolls every item PG hasn't seen: check ProfessionGear.log as `full-base/pg-50-fullbase-survey.txt`
describes (roll count, no exceptions, health ok after 2 min). Never `build "Biofuel Distillery"` (crash).
PG commands (`pg_*`) take character names only, not `#serial/index` handles.

### Launch 5 — Testing-Save-Full-Base (Beaks + Avarek), Forced + InGameTest (balance rows 161-199 + 89)

Needs: harness **33087EDE** (KAH 24 balance commands + `towns`; repo 8116c46) and the harness repo's `client/kah.py`
(its LONG_COMMANDS lets `detecttime`/`healtime`/`swimtime` wait 200 s; the 5090's stobe-auto uses it; the 4080 needs
`C:\KAH\client\kah.py` refreshed from the repo). PG: any build since 7D80DBB3 (the installed D7A60E49 is fine; 266C68F5
adds `pg_lootscan` for launch 7). Fresh copy (`load <kah copy>`) before every file: they research, craft, build and
change skills. Bench names come from the save itself (Weapon Smith V, Clothing Bench, Crossbow Crafting Bench,
Robotics Bench, Cooking Stove, Research Bench VII, Wheat Farm L, Prisoner Cage x12), so pg-50 is optional now.

| Order | File | Rows | Kind | Time (real) |
|---|---|---|---|---|
| 1 | `full-base/pg-56-critical-craft.txt` (**PG D7A60E49+**: `pg_force_critical`) | 89 | 2 crafts | ~3 min |
| 2 | `full-base/pg-88-balance-chances.txt` (all paused) | 196, 197, 198 | game's chance x45 | ~2 min |
| 3 | `full-base/pg-84-balance-medic.txt` | 191 | healtime x16 | ~10 min |
| 4 | `full-base/pg-87-balance-stealth.txt` | 195 | detecttime x16 | <= 35 min (120 s timeout) |
| 5 | `full-base/pg-89-balance-perception.txt` | 199 | detecttime x16 | <= 35 min |
| 6 | `full-base/pg-85-balance-athletics.txt` | 193 | timed 40 m run x16 | ~5 min |
| 7 | `full-base/pg-86-balance-swimming.txt` | 194 | findwater + swimtime x16 | ~5 min |
| 8 | `full-base/pg-83-balance-engineering.txt` | 185 | construct, 5 game min x16 | ~10 min |
| 9 | `full-base/pg-52-balance-weapon-smithing.txt` | 188 | craft, 10 game min x16 | ~18 min |
| 10 | `full-base/pg-53-balance-armour-smithing.txt` | 189 | craft | ~18 min |
| 11 | `full-base/pg-80-balance-crossbow-smithing.txt` | 190 | craft (Junkbow) | ~18 min |
| 12 | `full-base/pg-81-balance-robotics.txt` | 186 | craft (Gears) | ~18 min |
| 13 | `full-base/pg-82-balance-cooking.txt` | 187 | craft (Dried Meat) | ~18 min |
| 14 | `full-base/pg-54-research.txt` | 184 | research progress x16 | ~18 min |
| 15 | `full-base/pg-90-balance-farming.txt` | 161-176 | pg_operate, 30 game min x18 | ~55 min |
| 16 | `full-base/pg-55-turret.txt` (diagnostic) | 192 | turret vs dummy | 3 min at 1x |
| 17 | `full-base/pg-51-tool-weapons.txt` | 145, 229 | | 2 min |

Every generated file (all `pg-8x`/`pg-90` and pg-52/53/54) comes from `tools/balance_driver.py gen`; edit the generator.
Each measured point ends with an `@echo BAL2,<row>,<profession>,<skill>,<bonus>,<gear>,<label>,<kind>,<window s>,<a>,<b>`
line. Data: `python3 tools/balance_driver.py csv <all .out files> > balance.csv`, then
`python3 tools/analyze_balance.py balance.csv` (per profession curves + the cross-profession block for rows 200-208).
Send the .out files to the PG agent; a file whose BAL2 lines are empty (`${T}`) needs a scenario fix, not a re-run.
The 4080 is fine for all of these (no Stobe needed); keep each file's results on one machine.

### Launch 7 — Testing-Save-Full-Base, NormalVerbose + Normal rules + empty sidecar (row 250, ruin loot)

Before: `set_test_mode.ps1 -Mode NormalVerbose -Rules Normal -Sidecar Empty`; after: `-Sidecar Restore` and back to
Forced + InGameTest. Needs PG **266C68F5** (`pg_lootscan`) and harness 33087EDE (`towns`).

| Order | File | Rows | Time |
|---|---|---|---|
| 1 | `full-base/pg-91-ruin-loot.txt` (chest corpus at the base, then the 3 nearest ruins/labs by teleport) | 250 | ~3 min |

Then `python3 tools/analyze_loot.py <that launch's ProfessionGear.log>` prints the row 250 verdict.

### Rows 151-152 — frame time with PG on vs off (5090 only, same session)

Launch 1 (m16, DLL 7D80DBB3) is done but on an older build and day; for a clean A/B run both now, back to back:
1. Kenshi closed: `set_test_mode.ps1 -Mode Forced -Rules InGameTest` (PG on). Launch auto-home on a `kah-*` copy,
   run `auto-home/pg-15-crowd-frametime.txt`, keep the .out as `pg-15-on.out`. `kenshi-ctl stop`.
2. Kenshi closed: `set_test_mode.ps1 -Mode Disabled` (sidecar untouched). Fresh `kah-*` copy of auto-home, launch,
   run the same file, keep `pg-15-off.out`; check ProfessionGear.log has `config enabled=0` and no `roll source=`.
3. `set_test_mode.ps1 -Mode Forced -Rules InGameTest` (or `-Mode Normal -Rules Normal` at the very end).
Pass: at 100 and at 300 spawned, avg fps on within ~10% of off and worst_ms on <= ~1.5x off (no >250 ms frames only
in the on run); ProfessionGear.log growth bounded (roll lines for the spawned gear only).

### Launch 6 — Testing-Save-Squin (Beak + Kint), NormalVerbose + Normal rules + empty sidecar

Before: `set_test_mode.ps1 -Mode NormalVerbose -Rules Normal -Sidecar Empty`; after: `-Sidecar Restore`.

| Order | File | Rows | Time |
|---|---|---|---|
| 1 | `squin/pg-60-squin-shops.txt` (first load of a fresh copy = the measured distribution) | 220-227 (218/219 recheck) | 2 min |
| 2 | `squin/pg-61-squin-restock.txt` | 217 | ~60 real min at 50x |
| 3 | `squin/pg-62-unique-npc.txt` (harness KAH 13) | 108 | 1 min + a save/load |

### Row 274 (two launches, auto-home, Forced + InGameTest)

1. Launch auto-home, run `auto-home/pg-57a-id-collision-setup.txt`; note `K=pgp...` from its @echo; `kenshi-ctl stop`.
2. `powershell -File C:\KenshiModding\Kenshi-Profession-Gear-Progression\tools\make_id_collision.ps1 -Key <K>`
3. `kenshi-ctl launch -Save kah-pg-collide`, run `auto-home/pg-57b-id-collision-check.txt`; `kenshi-ctl stop`.
4. `make_id_collision.ps1 -Restore`; delete the save `kah-pg-collide`.

### Harness 5798EDF5 rows (auto-home, Forced + InGameTest, fresh copy each; run on the 4080 rig)

| File | Row | Note |
|---|---|---|
| `auto-home/pg-70-drop-pickup.txt` | 50 | |
| `auto-home/pg-71-melee-stats.txt` | 84 | |
| `auto-home/pg-72-unload-reload.txt` | 106 | |
| `auto-home/pg-73-pack-weight-edge.txt` | 120 | |
| `auto-home/pg-74-walktime.txt` | 254 | +x from Shay must be open for 40 m |
| `auto-home/pg-58-newgame-import.txt` | 132, 133, 240 | throwaway: saves `kah-pg-import-src`, then New Game + Import; delete that save after; run last |
