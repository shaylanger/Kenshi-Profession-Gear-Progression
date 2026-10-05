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
| PASS-live | 182 |
| PENDING-auto | 24 |
| NEEDS-SETUP | 30 |
| NEEDS-SHAY | 10 |
| DEFERRED | 26 |
| total | 343 |

## Balance validation gate (m28-m29, 2026-10-04, 4080 rig; this section supersedes the m21 notes)

Rule (RUN_ORDER.md "Gate"): no balance matrix runs before its profession's gate passes. Gate = 4 points on the
matrix fixture/method (`tools/balance_driver.py gate` -> `tests/ingame/full-base/gate/pg-gate-*.txt`): g1 skill 25 no
gear, g2 skill 25 + own +50%, g3 skill 90 no gear, g4 skill 25 no gear (drift bracket). `balance_driver.py csv` +
`gatecheck` print one GATE line per profession; every point carries native evidence (pg_bonus base/vanilla/effective,
harness `stat` mod= condition factors, game-clock elapsed). "Gear applies" (mechanical, pg_bonus match=1) is tracked
apart from "balance curve accepted".

**Invalidating finding (m29):** a HARNESS bug, not the save: `protect <npc> on` and `health <npc> 100` wrote the body
parts' stun damage (`fleshStun`) to max, so the game's derived part health was 0 and its wounds factor dropped to 0.25
on every craft skill (0.55 Medic, 0.665 Perception, run speed capped at 11) for ANY protected character, at "full" hp.
(First read as "Avarek is crippled in Full-Base": wrong, Beaks showed the same right after protect on.) Every balance
point of batches 17-20 measured a quartered skill (the m28 "x0.55 -> x0.25 drift" was this factor settling after the
first protect tick). Fixed in harness 6aa5685 (stun damage cleared; live: wounds=1.0000 with protect on, after
health 100, over 3.4 game hours). Kept from 0da79eb: `stat <worker> <stat> ~ wounds=1.0000` before every measurement
(it caught this); worker = Beaks with Avarek pinned (harmless). Batch 17-20 balance numbers are kept only as provisional
direction; none is a fitted value.

### Gate verdicts m31-m34 (2026-10-04; newest first wins over the older notes below)
Mechanical = gear reaches the formula the game uses; balance = gear gain matches the skill curve (gatecheck PASS).
- engineering_fs (83, 4080, PG 5DECDFA9 construction hook): PASS, gear +22.4% vs pred +22.8% (s25 3.78, s25+g 4.53, s90 8.09).
- medic_fs (84, 4080, PG 3CAEEB30 medic hook v3): PASS, gear +44.6% vs pred +10.5% (s25 11.3, s25+g 17.1, s90 18.3):
  the healing curve saturates, so +50% gear brings skill 25 close to skill 90 speed (balance note for Shay).
- farming (90, 4080, daylight 10:37-13:00): PASS, gear +101.7% vs pred +88.6% (JobOperateScaling), drift 2.2%.
- lockpicking (88/88fs/92/92fs, 5090 m31a): PASS with the game's exponential chance (gatecheck a8727aa): 88 gear
  +138.1% vs pred +137.8%, 92 +41.4% = pred (cap 0.9).
- thievery 88: PASS (+50% = pred). assassination_fs 88: PASS (+24.9% = pred); non-fs gear 0 (raw member, by design).
- stealth (87, 5090 m31b): fs PASS (gear +33.3% vs pred +41.7%); non-fs PASS skill, gear unresolved (raw member).
- perception (89): FAIL flat at d100 (5090), d180/d500/d800 (4080): seen in 1.1-2.2 s at every Perception and
  distance. Native evidence (4080 m34 pg-diag-perc-watch): a DR0 watchpoint on CharStats::perception (+0x8C) of the
  observer had 0 hits during detection and getStat(Perception) is not called per frame; observer_perception_mult
  stays 0.5 at Perception 25 and 90. Game behaviour: 1.0.65 sneak detection does not read the observer's
  Perception here, so neither skill nor gear can move it. Gear reaches getStat (eff 29 -> 43.5) = mechanical PASS
  for getStat only; balance not measurable (Shay decision whether Perception gear keeps a purpose).
- athletics_fs (85, 4080 m34, PG 3CAEEB30, harness 2B51AF08): PASS, run speed s25 69.5, s25+g 78.6, s90 85.4, drift
  2.4%; gear +14.5% vs linear pred +4.7% (m29b same: +14.6/+5.6): the run-speed curve is steep at low skill, so gear at
  s25 buys more than the average skill point. swimming_fs (86, 5090 m29b): PASS (gear +38% vs pred +19%).

### Mechanical passes (gear applies; native pg_bonus match=1, effective = vanilla x (1 + bonus))
- weapon/armour/crossbow smithing, robotics, cooking, science, thievery: pg_bonus match=1 at own +25/+50%
  (batches 19/20 + m29 Avarek gate). Thievery chance: own50 at s50 = s75 value (gear reaches the chance).
- assassination, stealth, swimming: hooked value changes only with FormulaScaling (the -fs files) because these
  formulas read raw CharStats members (design: PG note; Shay decision whether non-fs should apply).

### Valid measurements awaiting analysis
- m29 gate with Avarek under the protect wounds bug (equal mod 0.25 on all 4 points, so internally comparable but not representative):
  weapon smithing PASS (skill +74%, gear +22% vs pred +14%), armour smithing FAIL gear (+3.0% vs pred +4.8%, within
  noise of a compressed curve). Superseded by the Beaks gate (m29b).

### Unresolved (rerun in the Beaks gate, classify there)
- engineering (row 185, construct): batch 20 own50 at s50 ~ +2% vs skill gain 9.1k->23.8k/h: product candidate
  (construction may read Engineering through an unhooked path).
- medic (row 191): flat at every Medic with the Basic kit (35.3 s: the Basic kit caps Medic), Standard kit ~4x faster
  (d3/d4 ~4.5 s at 90 vs ~12.4 s at 10: scales; balance_driver ce5acc2 measures with the Standard kit); the protect wounds-bug medic factor 0.55
  was in play; d4 (Medic 10, Standard kit) next to d3 decides kit-bound vs skill.
- athletics (193): run speed 11.0 at every skill = the protect wounds-bug cap (harness 6aa5685, setup).
- swimming (194): actual swim speed capped at 4.0 while max_swim_speed rises: the protect wounds-bug cap again (setup), recheck.
- stealth (195): seen in ~1.5 s at 20 m at every skill: measurement insensitive (observer pinned facing at 20 m).
- lockpicking (196, Prisoner Cage / owned shackles): chance 0.0000 at every skill: measurement (harness), root cause found m31:
  KenshiLib's `Character::getLockpickChance` is a dead stub in the running exe (`RE_Kenshi\kenshi_x64.exe`, Steam 1.0.65,
  RVA 0x884DC0: reads the stat, returns 0.0). The game's real lockpick chance is the DoorLock chance fn (Steam RVA
  0x297D80, called from the lockpick task 0x359100 -> attempt 0x297E10): 0.9 if lock level <= skill, else
  0.9 / 2^((level - skill) / 10); skill = getStat(LOCKPICKING) (the PG getStat hook applies the gear in both
  FormulaScaling modes) - 10 when caged. Harness 2B51AF08+ `chance lockpick` calls that fn (`method=game`), the
  88/92 points assert it. **PG `HookLockpick` (src/ProfessionGearPlugin.cpp) hooks the dead stub = a no-op**
  (harmless; lockpicking gear still works through getStat; fs == non-fs expected). Remove that hook in a later
  plugin change (not done now: the file was being edited for the engineering construction hook). A cage whose
  lock level <= skill reads a flat 0.9: real game behaviour, use a higher-level lock for the curve.
- farming (161-176, pg-90): crop/water state dominates (1.3k..6.8k per window): measurement. m44 (5090): 30-min farm windows are crop-capped (a ripe field = ~4 straw, cleared in 0.45-0.9 h); balance_driver now waits for a ripe crop and measures 12-min windows (bc3e687), gate m42/out-i4 PASS skill +78.6% gear +39.9%. The 4080 m35/m41 farm numbers used 30-min windows: re-measure is Shay's call. m45 (5090) per-call method: per-call amount is deterministic (s25 0.728, s90 1.310); Shay set FarmOperateBonusFactor 0.5 (ad4d969). m46 gate (4080, `C:\KenshiTestRuns\m46\gate-4080.out`) PASS: s25 0.732, s25+own50 1.047 (+43.8%), s90 1.310, drift 0.9%.
- perception (199, pg-89): never ran (batch 20 stopped).
- pg-55 turret (192): the turret never picks a pinned dummy by itself (0 shots); harness `turret ... aim` designates it (dd72fa7). PASS 86/0 in batch W.
- pg-51: PASS 67/0 in batch T (2026-10-04).

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
| 89 | PASS-live | `C:\KenshiTestRuns\m22\out-t\pg-56-critical-craft-5090.scenario.txt` 101/0, batch T 2026-10-04 (archive/test-run-2026-10-03-m22.md) |
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
| 108 | PASS-live | `C:\KenshiTestRuns\m18-4080\pg-62-unique-npc-4080.out` 19/0 (4080, Squin, PG D7A60E49): unique NPC Ruka, census identical before/after save+load (npc records=1 rolled=0), no duplicate record. Weak: Ruka carries nothing that rolled |
| 109–116 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 117 | PASS-live | LIVE_TEST_PROGRESS.md "Generic backpack live weight control - PASS" (rechecked in pg-04) |
| 118–119 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 120 | PASS-live | `tests/ingame/auto-home/pg-73-pack-weight-edge.txt` 26/0, 5090 m36 batch X (`C:\KenshiTestRuns\m22\out-x\pg-73-packweight.txt`) |
| 121–122 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 123–124 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 (restart) |
| 125 | PASS-live | LIVE_TEST_PROGRESS.md Phase 1 live scan: no sidecar at the first launch, created safely |
| 126 | PASS-live | `C:\KenshiTestRuns\m6\pg-40.out` (archive/test-run-2026-10-03-m5.md) launch 4: log "loaded affixes=0 legacyIgnored=0" |
| 127–128 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 129–130 | PASS-live | ProfessionGear.log 2026-10-02 23:39: "loaded affixes=10157" (thousands of rows for items that no longer exist), normal startup, no errors |
| 131 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 133 | PASS-live | `tests/ingame/auto-home/pg-58-newgame-import.txt` steps 14-39, 5090 m37 batch I2: source save with 2 known records (K Medic:12 shirt, K2 Robotics:24.7 hat), `newgame` Wanderer reached the world, ProfessionGear.log after the save has no `restore` for K/K2 and no base_mismatch/collision; census shows only the new character's fresh roll (`C:\KenshiTestRuns\m22\out-i2\pg-58-i3-menu.txt`) |
| 132 | CLOSED (vanilla bug, Shay D7 2026-10-05) | Import into a fresh new game crashes Kenshi (kenshi_x64+0x94d6db AV read 0x270) with and without the PG DLL and on the 4080 without Stobe/KenshiFP (dump `C:\KenshiTestRuns\crash-m37-i4-nopg\`): a game bug, not PG |
| 134 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 135–141 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 142–144 | PASS-live | `C:\KenshiTestRuns\scenarios\smoke.txt` 2026-10-02 23:47: Stobe + KenshiFP + PG + harness in one game (help lists fp_mode/pg_bonus/stobe_say; stobe_ping, fp_state, pg_bonus answer) |
| 145 | PASS-live | `C:\KenshiTestRuns\m22\out-t\pg-51-tool-weapons-5090.scenario.txt` 67/0, batch T 2026-10-04 (Sickle roll tagged TOOL_FARMING) |
| 146 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 147 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 148 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 149 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 150 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 151–152 | PASS-live | `tests/ingame/auto-home/pg-15-crowd-frametime.txt`, 5090 same session, m36 batch X (PG Forced) vs Y2 (PG Disabled, `config enabled=0`, 0 rolls): 100 spawned avg 56.7 vs 59.7 fps (-5%), worst 232.4 vs 177.7 ms (1.31x); 300 spawned avg 48.9 vs 51.2 (-4.5%), worst 169.6 vs 167.7 (1.01x); pass = within ~10% / <=1.5x (`C:\KenshiTestRuns\m22\out-x\pg-15-on.txt`, `out-y2\pg-15-off.txt`) |
| 153–154 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 155 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 156 | PASS-live | `tests/ingame/auto-home/pg-09-soak.txt` 24/0, 5090 m36 batch X: 30 real min at 50x, health ok, forced Farming 20% unchanged at the end; a Hungry Bandit raid hit, protect held (`C:\KenshiTestRuns\m22\out-x\pg-09-soak.txt`) |
| 157 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 158 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 159 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 160 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): one crafted roll for the craft saved mid-progress |
| 161–176 | PASS-live (mechanical); balance FAIL row 206 | 4080 matrix m35 (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`, m35.csv): Farming s10 0.364 / s90 0.728, +50% gear +108.7%; s25+50% (0.888) beats s75 (0.675) = gear too strong (finding for Shay) |
| 177 | PASS-live | `C:\KenshiTestRuns\m16\pg-12-job-throughput.out` (v5) 50/0 on DLL 7D80DBB3 + harness F946C881, Manual Stone Processor, 3 x 30 game min: output_progress A no gear 0.1042, B +50% Labouring (default build, JobOperateScaling on) 0.1599 (x1.53), C same gear with scaling off 0.1095 (x1.05, ~A). Jobs read the raw skill; the operate scaling makes equipped Labouring count. (m16 v4 on 10C19BAB: 0.1065 / 0.1123 / 0.1695; m13/m14 powered Stone Mine runs at machine rate) |
| 178–183 | PASS-live | `C:\KenshiTestRuns\m16\pg\pg-14-labouring-curve.out` 119/6 (DLL 7D80DBB3; the 6 FAILs were the per-window stone top-up `fill` on a full machine, removed since; every window calls>0, state=NORMAL, stone 19-20). output_progress per 30 game min: no gear skill 10 0.0842, 25 0.0814, 50 0.0973, 75 0.1142, 90 0.1061 (avg amount/call 0.52 -> 0.70: skill counts weakly, ~5% window noise); skill 50 with +10% 0.1135 (x1.17 of L50), +25% 0.1222 (x1.26), +100% 0.2392 (x2.46), set 25+25 0.1502 (x1.54, like pg-12 B x1.53). Gear scales as designed; repeats for the final fit still to do |
| 184 | PASS-live (mechanical) | 4080 m35 science: s10 21.5 / s90 47.7, +50% gear +23.1%; lab50 control 0.75 off (noise or Labouring leak, balance pass) (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 185 | PASS-live (mechanical) | 4080 m35 engineering_fs: s10 2.90 / s90 8.39, +50% gear +25.3% (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 186–187 | PASS-live (mechanical) | 4080 m35 robotics s10 0.021 / s90 0.048, +50% +22.0% (lab50 ctl 0.84); cooking s10 0.127 / s90 0.287, +50% +23.6% (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 188 | PASS-live (mechanical) | 4080 m35 weapon smithing: s10 0.034 / s90 0.077, +50% gear +25.6% (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 189 | PASS-live (mechanical) | 4080 m35 armour smithing: s10 0.024 / s90 0.054, +50% gear +27.7% (`C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 190 | PASS-live; balance done (Shay D5 2026-10-05) | 5090 m41 pg-80 695/0: crossbow +7.1% / +14.1% at +25 / +50% gear (0.283/pt), lab50 ctl 1.000 (no Labouring leak; the m35 +99.6% / ctl 1.60 is gone) (`archive/test-run-2026-10-05-m41.md`) |
| 191 | PASS-live (Shay D2 2026-10-05: medic gear raises kit quality) | 4080 m43 pg-84-balance-medic-fs 479/0 (PG 06A18B15, clamp 6856412): +22.1% / +41.5% heal rate at +25 / +50% gear (0.83/pt), lab50 ctl 1.000, s75 > s25+50% (16.8 / 15.8); 5090 probe: kit 40 -> 60 at Medic >= 40, held below (`C:\KenshiTestRuns\m43 80\matrix-analysis.txt`). Medic balance runs use the `-fs` variant (formulas on) |
| 192 | PASS-live (mechanical); balance accepted (Shay D6 2026-10-05) | `C:\KenshiTestRuns\m22\out-w\pg-55-turret-5090-r6.scenario.txt` 86/0, batch W 2026-10-04: Beaks on the turret, gun skill01 0.10 / 0.90 / 0.15 (+50% gear), dummy worst 5% / -27% KO / -58% KO |
| 193–198 | PASS-live (mechanical) | 193 athletics_fs: 4080 m34 + 5090 batch T (`C:\KenshiTestRuns\m22\out-t\pg-85-balance-athletics-fs-5090.scenario.txt` 323/0: swim 300 m s10 4.3 s / s90 3.0 / own50 ~2.8); 194 swimming s10 2.38 / s90 30.9 +24.8%; 195 stealth s10 0.70 / s90 1.30 +16.7%; 196-198 lockpick s25+50% 0.48, stealth KO (assassination) +36.7%, thievery +50% (4080 m35, `C:\KenshiTestRuns\pgbal-4080\m35-analysis.txt`) |
| 199 | REMOVED (Shay D3 2026-10-05) | Perception dropped from gear (no Perception affixes, goggles/scout gear roll other stats; detection ignores perception in 1.0.65, m37 pg-89 flat 1.4-1.9 s); confirmed m41 pg-02 115/0 + pg-03 44/0 |
 |
| 200–210 | PASS-live; 206 PASS (Shay D1 2026-10-05); farm factor now 0.5 (m46 gate PASS, gear +43.8% at +50%, per-call method) | 4080 m41 farming (PG 9517FF06, farm factor 0.15): +25% gear +16.5%, +50% +29.7% at s50 (0.59/pt), s90/s10 2.23, lab50 ctl 1.011, 206 s75 > s25+50% (0.643 / 0.466); crossbow/robotics/science m41 0.28-0.47/pt (`archive/test-run-2026-10-05-m41.md`) |
| 211 | PASS-live | LIVE_TEST_PROGRESS.md "Real trader-stock context - PASS" (Blamo) |
| 212 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 213 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 214–216 | PASS-live | LIVE_TEST_PROGRESS.md "Trader item transfer / purchase-style identity - LIVE PASS" |
| 217 | PASS-live | `C:\KenshiTestRuns\m18-4080\pg-61-squin-restock-4080.out` 37/0 (+ `-guard` variant 37/0; 4080, PG D7A60E49): after 2 game days Trader Tentpeg eligible stock 7 -> 4, no key changed its affixes, new stock rolled on first sight. The watched Athletics item was sold/removed in both runs, so the "survivor keeps its affixes" half held only for the items that stayed |
| 218–219 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 220–227 | PASS-live (measured; finding for Shay) | `C:\KenshiTestRuns\m18-4080\pg-60-squin-shops-4080.out` 53/15 + `pg-63-squin-shopkeepers-4080.out` 22/0 (4080, Normal chances, empty sidecar; the 15 FAILs are trader slots 8-12: Squin has 7 traders within 300 m). Squin shops: 7 shopkeepers, eligible stock only at Trader Tentpeg (7) and Double (3, armour), 0 at the barman and 3 apothecaries; census npc 210-247 records, 112-113 rolled: Athletics 53-57, Perception 47-54, Medic 22-27 (222 measurable), Engineering 2 (223), Robotics 1 (225), Stealth/Lockpicking/Thievery 2-3; traders 97 records, 35 rolled. Farming (220), Labouring (221), Science (224), Cooking (226), Smithing (227): 0 in Squin (also ~0 in the Trader town, m6). Production gear practically never appears in shops: a rarity/source decision for Shay (no balance change until the balance data is complete) |
| 228 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 229 | PASS-live | `C:\KenshiTestRuns\m22\out-t\pg-51-tool-weapons-5090.scenario.txt` 67/0, batch T 2026-10-04 (Farming 20 Sickle: 0% carried, 20% equipped, 0% unequipped) |
| 230 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 231–234 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 235–236 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 237 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 238 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 239 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 240 | PASS-live (new game half) | batch I2: trader near the new start (`pg_shop` trader=1), no persistent-id collision for K/K2 after `newgame`; the import half is blocked by the vanilla import crash (see 132) |
| 241–243 | PASS-live | LIVE_TEST_PROGRESS.md "Exact-ID harness + world-loot live results" |
| 244 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 245 | PASS-live | `C:\KenshiTestRuns\m6\pg-41.out` (archive/test-run-2026-10-03-m5.md) launch 4: 30 world-loot Rattan Hats (tier 6, chance .96 x .5 = .48) rolled 15/30 = .50 |
| 246–249 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 250 | PASS-live | `tests/ingame/full-base/pg-91-ruin-loot.txt` batch Z2 (m39, out-z2, kah-fullbase, NormalVerbose + Normal + empty sidecar; PG 81D89609, lootscan scans furniture): 5 Drifters chests, scan items=45 eligible=24 rolled=10; re-scan already_recorded=24 (=eligible), rolled unchanged; `tools/analyze_loot.py` verdict PASS (share 0.37 in 0.15..0.60, 12 stats, top 16%). Own base benches in the radius correctly skipped (skipped_own=2). Note: the 3 nearest ruins had no filled containers (items=0) |
| 251–252 | PASS-offline | core unique/legendary tests; a live check needs a unique item instance in a fixture |
| 253 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): high-quality crafted items roll |
| 254 | PASS-live (mechanical); feel = Shay (D4) | run speed: pg-74-walktime 34/0 (m36) max 81.8 -> 113.1 u/s at +50% Athletics. Acceleration (Shay D4, PG f212cbc: updateVelocity clamp + HavokCharacter::update braking curve scaled): pg-94 m43 batches F/G/H on screen 12/12: t50 0.300 vs 0.400 s (0.74-0.75) PASS, top speed unchanged PASS; t90 and stop distance (move order or halt) unchanged: those come from the path slow-down / order delay / halt routine, not the acceleration value (`C:\KenshiTestRuns\m42\out-g`, `out-h`) |
| 255 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 256 | PASS-live (negative) + PASS-offline (positive) | the mod stack has no swim-named item (gamedata/rebirth/Newwworld/UWE/patch .mod strings: only `swim speed mult`/`swimming mult` fields; the SWIM_GEAR tag needs swim/diving/diver/flipper/wetsuit in the name), and 554,143 `roll source` lines in 365 archived ProfessionGear.logs (C:\KenshiTestRuns\logs, m37) contain 0 Swimming affixes and 0 SWIM_GEAR tags = Swimming never lands on invalid gear; the positive side (a Diving Suit gets SWIM_GEAR/Swimming) is the offline classifier test `tests/test_profession_gear.cpp`, and a forced Swimming affix applies live (194). No water-route run possible without swim gear in the stack |
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
