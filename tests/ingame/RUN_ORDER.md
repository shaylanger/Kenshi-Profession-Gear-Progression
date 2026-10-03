# PG in-game run order (for the coordinator)

Build: `out\ProfessionGearProgression.dll` **59EFB4B1** (`install-dll.ps1 ProfessionGear`, Kenshi closed).
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
| 8b | auto-home | `auto-home/pg-12-job-throughput.txt` (row 177, ~4.5 game hours at 10x; needs harness C5804B0C) | fresh |
| 9 | auto-home | `auto-home/pg-09-soak.txt` (30 real minutes at 50x; optional, last) | fresh |
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
