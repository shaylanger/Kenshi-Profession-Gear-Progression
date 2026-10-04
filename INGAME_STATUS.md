# Profession Gear Progression — in-game test status

**Updated:** 2026-10-03 (after coordinator runs m2-m16). One row per `TEST_PLAN.md` ID (1–331 plus 16a–16c and 30a–30i = 343 rows).
The coordinator session runs the game; the PG agent writes scenarios (`tests/ingame/`, order in
`tests/ingame/RUN_ORDER.md`) and fixes. Build: **ProfessionGearProgression.dll 7D80DBB3** (FAA5B471 ran m4-m12).

States: **PASS-offline** (core tests / build gates / source contracts), **PASS-live** (with evidence),
**PENDING-auto** (a scenario file exists; the harness can run it without Shay), **NEEDS-SETUP**
(missing situation/fixture/command, named in the row), **NEEDS-SHAY** (eyes/feel only),
**DEFERRED** (by design).

| State | Rows |
|---|---|
| PASS-offline | 71 |
| PASS-live | 181 |
| PENDING-auto | 25 |
| NEEDS-SETUP | 30 |
| NEEDS-SHAY | 10 |
| DEFERRED | 26 |
| total | 343 |

## Rows

| ID | State | Evidence / scenario / what is missing |
|---|---|---|
| 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16a, 16b, 16c, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 30a, 30b, 30c, 30d, 30e, 30f, 30g, 30h | PASS-offline | run_tests.bat (core), build/export/package gates; 4 as amended by 307 |
| 30i | PASS-offline | INSTALLED_MOD_GEAR_AUDIT.md + 2026-10-03 weapon-description corpus scan, turned into regressions (322) |
| 31–41 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 1 - automated startup PASS": all hooks "hooked", world loads |
| 42–43 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 44 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2 persistence: saves pg-phase2-equipped / pg-persist-v2 without crash |
| 45 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2: sidecar reloaded after a full Kenshi restart (318) |
| 46–48 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 2 equipped-section rerun" + "Phase 2 - equip-only + persistence core PASS" |
| 49 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 50 | PASS-live | `C:\KenshiTestRuns\m16-4080\pg-70-drop-pickup.out` 16/0 (harness 5798EDF5): same key + affixes after drop/pickup, also picked up by another character |
| 51–56 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 57 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 58 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 59–62 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 63 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2 + `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) (all 17 stats match=1) |
| 64 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 65–66 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2 + `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) (all 17 stats match=1) |
| 67–79 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 80 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2 + `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) (all 17 stats match=1) |
| 81–82 | PASS-live | `C:\KenshiTestRuns\m4\pg-08-job-path.out` (archive/test-run-2026-10-03-m4.md) 39/0: base Labouring 50.00 -> 50.43 while geared (+50%), unchanged by removing the gear |
| 83 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 84 | PASS-live | `C:\KenshiTestRuns\m16-4080\pg-71-melee-stats.out` 26/0: combat stats identical with +100% profession affixes on two items |
| 85–86 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 87 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): crafted tiers rise with Armour Smithing (5 vs 100; Kenshi quality has a random spread) |
| 88 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 89 | PENDING-auto | `tests/ingame/full-base/pg-56-critical-craft.txt` (needs PG D7A60E49: test-only `pg_force_critical on` hooks CraftingBuilding::calculateCriticalChance -> 1.0; real Sickle craft normal vs forced critical) |
| 90–92 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 93 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 94 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 95–97 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 98–100 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 101 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 102 | PASS-live | core rate test + live launch 4 (Normal chances, m6 pg-40): 10 spawned slaves 9/73 items rolled (12%) vs 10 drifters 16/32 (50%) |
| 103–104 | PASS-offline | core rate tests over 4,000 instances (slave/poor x0.2, matching specialist x1.35); 103 needed fix 2b14a4c; live data for 102 in config/pg-40 |
| 105 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 106 | PASS-live | `C:\KenshiTestRuns\m16-4080\pg-72-unload-reload.out` 14/0: same key + affixes after unload/reload, restored not rerolled |
| 107 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 108 | PENDING-auto | `tests/ingame/squin/pg-62-unique-npc.txt` (harness KAH 13 `unique=1`; census for the first unique NPC identical across save/load) |
| 109–116 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 117 | PASS-live | LIVE_TEST_PROGRESS.md "Generic backpack live weight control - PASS" (rechecked in pg-04) |
| 118–119 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 120 | PENDING-auto | `tests/ingame/auto-home/pg-73-pack-weight-edge.txt` (harness 5798EDF5 `packweight` inventory/ground/chest/nested) |
| 121–122 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 123–124 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 (restart) |
| 125 | PASS-live | LIVE_TEST_PROGRESS.md Phase 1 live scan: no sidecar at the first launch, created safely |
| 126 | PASS-live | `C:\KenshiTestRuns\m6\pg-40.out` (archive/test-run-2026-10-03-m5.md) launch 4: log "loaded affixes=0 legacyIgnored=0" |
| 127–128 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 129–130 | PASS-live | ProfessionGear.log 2026-10-02 23:39: "loaded affixes=10157" (thousands of rows for items that no longer exist), normal startup, no errors |
| 131 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 132–133 | PENDING-auto | `tests/ingame/auto-home/pg-58-newgame-import.txt` (harness 5798EDF5 `newgame`/`import`; 132 is a policy finding: IDs persist or import rerolls) |
| 134 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 135–141 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 142–144 | PASS-live | `C:\KenshiTestRuns\scenarios\smoke.txt` 2026-10-02 23:47: Stobe + KenshiFP + PG + harness in one game (help lists fp_mode/pg_bonus/stobe_say; stobe_ping, fp_state, pg_bonus answer) |
| 145 | PENDING-auto | `tests/ingame/full-base/pg-51-tool-weapons.txt` (Testing-Save-Full-Base: research UWE utility weapons, craftfinish a Sickle; roll line tags TOOL_FARMING) |
| 146 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 147 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 148 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 149 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 150 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 151–152 | PENDING-auto | `tests/ingame/auto-home/pg-15-crowd-frametime.txt` (harness `fps` worst_ms/avg with 100 and 300 spawned characters), run in launch 1 (PG on) and launch 3 (PG disabled) and compare. Launch 1 done: `C:\KenshiTestRuns\m16\pg\pg-15-crowd-frametime.out` 56/0 (DLL 7D80DBB3): baseline avg 57.5 fps, 100 spawned avg 56.8 worst 148 ms, 300 spawned avg 44.2 worst 155 ms; launch 3 (PG off) still to run |
| 153–154 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 155 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 156 | PENDING-auto | `tests/ingame/auto-home/pg-09-soak.txt` (30 real min at 50x, Shay and Malzin protected) |
| 157 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 158 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 159 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 160 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): one crafted roll for the craft saved mid-progress |
| 161–176 | PENDING-auto | `tests/ingame/full-base/pg-90-balance-farming.txt` (generated by `tools/balance_driver.py`: Wheat Farm L, pg_operate output_progress per 30 game min; Farming 10/25/75/90 without gear, 3 x none/+25%/+50%/Labouring control at 50, sanity 75 vs 25+50%). Farming x1.50 with +50% already measured (row 331) |
| 177 | PASS-live | `C:\KenshiTestRuns\m16\pg-12-job-throughput.out` (v5) 50/0 on DLL 7D80DBB3 + harness F946C881, Manual Stone Processor, 3 x 30 game min: output_progress A no gear 0.1042, B +50% Labouring (default build, JobOperateScaling on) 0.1599 (x1.53), C same gear with scaling off 0.1095 (x1.05, ~A). Jobs read the raw skill; the operate scaling makes equipped Labouring count. (m16 v4 on 10C19BAB: 0.1065 / 0.1123 / 0.1695; m13/m14 powered Stone Mine runs at machine rate) |
| 178–183 | PASS-live | `C:\KenshiTestRuns\m16\pg\pg-14-labouring-curve.out` 119/6 (DLL 7D80DBB3; the 6 FAILs were the per-window stone top-up `fill` on a full machine, removed since; every window calls>0, state=NORMAL, stone 19-20). output_progress per 30 game min: no gear skill 10 0.0842, 25 0.0814, 50 0.0973, 75 0.1142, 90 0.1061 (avg amount/call 0.52 -> 0.70: skill counts weakly, ~5% window noise); skill 50 with +10% 0.1135 (x1.17 of L50), +25% 0.1222 (x1.26), +100% 0.2392 (x2.46), set 25+25 0.1502 (x1.54, like pg-12 B x1.53). Gear scales as designed; repeats for the final fit still to do |
| 184 | PENDING-auto | `tests/ingame/full-base/pg-54-research.txt` (now generated: Research Bench VII, several techs queued, research status progress per 10 game min, 16 points) |
| 185 | PENDING-auto | `tests/ingame/full-base/pg-83-balance-engineering.txt` (harness 33087EDE (KAH 24) `construct` = an unfinished Small Shack with its materials in + BUILD job, `construction` = progress per 5 game min, 16 points) |
| 186–187 | PENDING-auto | `full-base/pg-81-balance-robotics.txt` (Robotics Bench, Gears from Iron Plates after `research "Robotics"`) and `pg-82-balance-cooking.txt` (Cooking Stove, Dried Meat from Raw Meat); craft queue drop per 10 game min, 16 points each |
| 188 | PENDING-auto | `tests/ingame/full-base/pg-52-balance-weapon-smithing.txt` (regenerated: Weapon Smith V, Sickle, 16 points incl. +25% and the sanity pair) |
| 189 | PENDING-auto | `tests/ingame/full-base/pg-53-balance-armour-smithing.txt` (regenerated: Clothing Bench, Rag Shirt) |
| 190–191 | PENDING-auto | 190: `full-base/pg-80-balance-crossbow-smithing.txt` (Crossbow Crafting Bench, Junkbow from Steel Bars + Hinge after `research "Crossbow Crafting"`); 191: `full-base/pg-84-balance-medic.txt` (harness 33087EDE (KAH 24) `healtime <medic> <patient> wound 30`: same three cuts every point, bandaging per game second) |
| 192 | PENDING-auto | `tests/ingame/full-base/pg-55-turret.txt` (diagnostic: hostile dummy hp after 45 s at Turrets 10 / 90 / 10+50% gear) |
| 193–199 | PENDING-auto | harness 33087EDE (KAH 24): 193 `pg-85-balance-athletics.txt` (40 m timed run + runspeed), 194 `pg-86-balance-swimming.txt` (`findwater` + `swimtime` through deep water), 195 `pg-87-balance-stealth.txt` and 199 `pg-89-balance-perception.txt` (`detecttime`: seconds until a neutral observer 20 m away sees the sneaking worker; sneaker vs observer skill varies), 196-198 `pg-88-balance-chances.txt` (the game's own `getLockpickChance` on a Prisoner Cage, `getStealthKOChance`, `getStealingSuccessChance` against a dummy; all paused, 45 reads) |
 |
| 200–210 | PENDING-auto | `tools/analyze_balance.py` cross-profession block (200 curves side by side, 201 universal-table spread, 202 per-profession bonus for each target band, 203-205 bands, 206 skill 75 vs skill 25 + 50% gear, Labouring control) on `tools/balance_driver.py csv` of every launch-5 .out; first data: Labouring (pg-14): +1% gear bonus = +1.09% throughput, while skill 10 -> 90 gives only x1.26 (+25% gear already beats the whole skill curve: principle 4 at risk; decide after the other professions) |
| 211 | PASS-live | LIVE_TEST_PROGRESS.md "Real trader-stock context - PASS" (Blamo) |
| 212 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 213 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 214–216 | PASS-live | LIVE_TEST_PROGRESS.md "Trader item transfer / purchase-style identity - LIVE PASS" |
| 217 | PENDING-auto | `tests/ingame/squin/pg-61-squin-restock.txt` (2 game days at 50x; a surviving key keeps its affixes; restock itself diagnostic) |
| 218–219 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 220–227 | PENDING-auto | `tests/ingame/squin/pg-60-squin-shops.txt` (own launch, NormalVerbose + Normal rules + empty sidecar; pg_shop + pg_census per Squin trader; types missing in Squin become "needs setup (no <type> shop in Squin)") |
| 228 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 229 | PENDING-auto | `tests/ingame/full-base/pg-51-tool-weapons.txt` (Sickle with Farming 20: 0% carried, 20% equipped, 0% unequipped) |
| 230 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 231–234 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 235–236 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 237 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 238 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 239 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 240 | PENDING-auto | `tests/ingame/auto-home/pg-58-newgame-import.txt` (trader stock after newgame/import) |
| 241–243 | PASS-live | LIVE_TEST_PROGRESS.md "Exact-ID harness + world-loot live results" |
| 244 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 245 | PASS-live | `C:\KenshiTestRuns\m6\pg-41.out` (archive/test-run-2026-10-03-m5.md) launch 4: 30 world-loot Rattan Hats (tier 6, chance .96 x .5 = .48) rolled 15/30 = .50 |
| 246–249 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 250 | PENDING-auto | `tests/ingame/full-base/pg-91-ruin-loot.txt` (own launch, NormalVerbose + Normal + empty sidecar; PG 266C68F5 `pg_lootscan` + harness `towns`): a Drifters-owned chest filled with 15 gear types x 3 at the base, then the 3 nearest ruins/labs by teleport; verdict `tools/analyze_loot.py` |
| 251–252 | PASS-offline | core unique/legendary tests; a live check needs a unique item instance in a fixture |
| 253 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): high-quality crafted items roll |
| 254 | PENDING-auto | `tests/ingame/auto-home/pg-74-walktime.txt` (harness 5798EDF5 `runspeed`/`walktime`) |
| 255 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 256 | NEEDS-SETUP | swim gear in the mod stack + a water route |
| 257 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 258–271 | DEFERRED | by design (Shay): FCS profession items and selective stacking are later goals |
| 272 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 273 | PASS-live | LIVE_TEST_PROGRESS.md 309 (Bolts) + 308-311; rechecked in pg-03 |
| 274 | PASS-live | `C:\KenshiTestRuns\m16-4080\pg-57a-id-collision-setup.out` 11/0 + `pg-57b-id-collision-check.out` 8/0 (4080 rig, PG D7A60E49): sidecar row with another base ID -> rebound to a new key, no marker affix, no base_mismatch, no stale Medic bonus |
| 275–276 | PASS-live | implicit in every live run (per-character bonus with several characters, inventory callbacks under heavy moves, sidecar atomic replace); the failure path of 279 is offline only |
| 277 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 278 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 279 | PASS-live | implicit in every live run (per-character bonus with several characters, inventory callbacks under heavy moves, sidecar atomic replace); the failure path of 279 is offline only |
| 280 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 281–282 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 283–288 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 289 | PASS-live | every ProfessionGear.log: version + normalized config + rule counts before the hook lines |
| 290 | DEFERRED | superseded: installs go through install-dll.ps1, no DLL backups (git is the history) |
| 291 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 292 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0; Malzin carried weight 17.9 kg with the ore pack worn vs 59.3 kg not worn: the game uses the hooked pack total |
| 293–297 | PASS-offline | classifier semantic-boundary and profession-pack tests |
| 298 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 299 | PASS-live | LIVE_TEST_PROGRESS.md generic pack weight half; Athletics half rechecked in pg-04 |
| 300 | PASS-live | verbose roll lines with source/key/baseId/name/tier/tags/affixes in every live log; rechecked in pg-02 |
| 301–305 | PASS-offline | classifier semantic-boundary and profession-pack tests |
| 306 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 307 | PASS-live | LIVE_TEST_PROGRESS.md RE_Kenshi loader contract |
| 308–311 | PASS-live | LIVE_TEST_PROGRESS.md "Live non-equippable regression" + "Exact-ID harness" (309) |
| 312–313 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 2 equipped-section rerun" |
| 314–318 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 restart |
| 319 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 320 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 321 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 322 | PASS-offline | core regressions added 2026-10-03 (bd6643c, 2b14a4c) |
| 323–324 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 325 | PASS-offline | core regressions added 2026-10-03 (bd6643c, 2b14a4c) |
| 326 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 327 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 328 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 329 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 330 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 331 | PASS-live | `C:\KenshiTestRuns\m16\pg-12-job-throughput.out` (v5) 50/0: default build gives x1.53 work with +50% Labouring, `pg_jobscale off` returns to ~x1.05 (same run as 177). Labouring path (`ProductionBuilding::operate`) measured. Farming path (`FarmBuilding::operate`): `C:\KenshiTestRuns\m16\pg\pg-13-farm-operate.out` 51/1 on Home's Wheat Farm XL, scaled/amount A no gear 195.44/195.44 = 1.00, B +50% Farming 360.56/240.38 = 1.50, C scaling off 211.09/211.09 = 1.00 (the 1 FAIL was the fixed-time warm-up check: no farm call in the first 15 game min, now an `@until`) |

## Notes

- PASS-live evidence predates the 2026-10-03 fixes (shop storage scan, purchase hook, weapon names,
  poor-NPC wealth, inactive stale records, CRLF sidecar). Those touch rows 101-104, 211-219, 237, 298,
  321-328; the PENDING-auto scenarios recheck the live ones (pg-03, pg-04, pg-21, pg-30).
- Scenario reply formats come from the current harness and PG commands (`pg_info`, `pg_check`,
  `pg_bonus` with `match=1`, `pg_pack` ratios, `pg_shop`, `pg_building`, `pg_take`, `pg_store`,
  `pg_loot`, `pg_census`). Unknowns that may need a one-line scenario fix on the first run are
  noted in each file's header (fixture item names, building names, auto-equip of given items).

## Live measurements, Normal chances (launch 4, run m6, Trader + auto-home, empty sidecar)

- Overall: 1,627 rolls in the session, 931 without an affix (57%), 43% with at least one.
- `pg_census` at the Trader town: NPC gear 240/589 rolled (Medic 83, Perception 87, Athletics 80,
  Stealth 26, Lockpicking 15, Engineering 3, Turrets 4, others <= 3); traders' own gear 94/176;
  natural shop stock 4/12 rolled (Athletics 1, Stealth 1, Lockpicking 2).
- Slaves 9/73 items rolled (12%) vs drifters 16/32 (50%): suppression about x0.25 (design x0.2). Row 102.
- World loot: 30 tier-6 Rattan Hats, 15 rolled (50%) vs expected 48%. Row 245.
- Observations for balance (rows 220-227, 237): the Trade Ninjas town stocks very little eligible
  gear (12 items in 5 shops, all stealth/athletic). Town NPC rolls lean to Medic/Perception/Athletics
  (generic workwear follows the wearer's top skill; many town NPCs' top skill is Medic). Production
  professions (Farming, Labouring, Science, Smithing) barely appear here: judge rarity on other shop
  types before changing chances.
