# PG in-game run order (for the coordinator)

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

### Launch 5 — Testing-Save-Full-Base (Beaks + Avarek), Forced + InGameTest

| Order | File | Rows | Time |
|---|---|---|---|
| 1 | `full-base/pg-50-fullbase-survey.txt` (send the .out to the PG agent: bench/turret names) | load test | 1 min |
| 2 | `full-base/pg-51-tool-weapons.txt` | 145, 229 | 2 min |
| 3 | `full-base/pg-52-balance-weapon-smithing.txt` (generated) | 188 | 11 x 10 game min at 10x, ~12 min |
| 4 | `full-base/pg-53-balance-armour-smithing.txt` (generated; needs a Clothing bench) | 189 | ~12 min |
| 5 | `full-base/pg-55-turret.txt` (diagnostic) | 192 | 3 min at 1x |
| 6 | `full-base/pg-54-research.txt` (harness KAH 14) | 184 | 5 x 10 game min, ~6 min |
| 7 | `full-base/pg-56-critical-craft.txt` (**PG D7A60E49**: `pg_force_critical`) | 89 | ~3 min |

Fresh copy per file (they research, craft and change skills). Balance data:
`python3 tools/balance_driver.py csv <out files> > balance.csv` then `python3 tools/analyze_balance.py balance.csv`.

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

### Waiting for harness commands (scenarios ready in `pending-harness/`, auto-home, Forced + InGameTest)

| KAH | File | Row |
|---|---|---|
| 15 drop/pickup | `pending-harness/pg-70-drop-pickup.txt` | 50 |
| 16 melee stat names | `pending-harness/pg-71-melee-stats.txt` | 84 |
| 17 unload/reload | `pending-harness/pg-72-unload-reload.txt` | 106 |
| 18 nested/unowned pack weight | `pending-harness/pg-73-pack-weight-edge.txt` | 120 |
| 19 walktime | `pending-harness/pg-74-walktime.txt` | 254 |
| 21 import/new game | not written yet (no command spec) | 132, 133, 240 |

Written against the spec before the commands existed: forward the built command replies and the PG agent adjusts the `~` patterns.
