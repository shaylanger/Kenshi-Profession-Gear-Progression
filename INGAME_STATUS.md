# Profession Gear Progression — in-game test status

**Updated:** 2026-10-03 (after coordinator runs m2-m13). One row per `TEST_PLAN.md` ID (1–330 plus 16a–16c and 30a–30i = 342 rows).
The coordinator session runs the game; the PG agent writes scenarios (`tests/ingame/`, order in
`tests/ingame/RUN_ORDER.md`) and fixes. Build: **ProfessionGearProgression.dll 59EFB4B1** (FAA5B471 ran m4-m12).

States: **PASS-offline** (core tests / build gates / source contracts), **PASS-live** (with evidence),
**PENDING-auto** (a scenario file exists; the harness can run it without Shay), **NEEDS-SETUP**
(missing situation/fixture/command, named in the row), **NEEDS-SHAY** (eyes/feel only),
**DEFERRED** (by design).

| State | Rows |
|---|---|
| PASS-offline | 71 |
| PASS-live | 169 |
| PENDING-auto | 1 |
| NEEDS-SETUP | 65 |
| NEEDS-SHAY | 10 |
| DEFERRED | 26 |
| total | 342 |

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
| 50 | NEEDS-SETUP | harness drop-to-ground + pick-up commands (storage moves exist: pg_store/pg_take, used for 52/246) |
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
| 84 | NEEDS-SETUP | harness `stat` has no melee attack/defence/dodge/toughness names (code passes unmapped stats through unchanged) |
| 85–86 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 87 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): crafted tiers rise with Armour Smithing (5 vs 100; Kenshi quality has a random spread) |
| 88 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 89 | NEEDS-SETUP | a way to force a critical-success craft (or many weapon crafts with a known crit chance) |
| 90–92 | PASS-live | `C:\KenshiTestRuns\m3\pg-10-craftfinish.out` (archive/test-run-2026-10-03-m3.md) 29/0 with the craft-output fix 159C552F (m2 found the bug) |
| 93 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 94 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 95–97 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 98–100 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 101 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 102 | PASS-live | core rate test + live launch 4 (Normal chances, m6 pg-40): 10 spawned slaves 9/73 items rolled (12%) vs 10 drifters 16/32 (50%) |
| 103–104 | PASS-offline | core rate tests over 4,000 instances (slave/poor x0.2, matching specialist x1.35); 103 needed fix 2b14a4c; live data for 102 in config/pg-40 |
| 105 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 106 | NEEDS-SETUP | a reliable stream-out/in trigger for one NPC (unload command, or a fixture + teleport distance known to unload) |
| 107 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 108 | NEEDS-SETUP | a named/unique NPC (isUnique) with eligible gear in a fixture, or a spawnable unique template |
| 109–116 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 117 | PASS-live | LIVE_TEST_PROGRESS.md "Generic backpack live weight control - PASS" (rechecked in pg-04) |
| 118–119 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 120 | NEEDS-SETUP | a weight readout for a pack that is nested or not owned by a character |
| 121–122 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 123–124 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 (restart) |
| 125 | PASS-live | LIVE_TEST_PROGRESS.md Phase 1 live scan: no sidecar at the first launch, created safely |
| 126 | PASS-live | `C:\KenshiTestRuns\m6\pg-40.out` (archive/test-run-2026-10-03-m5.md) launch 4: log "loaded affixes=0 legacyIgnored=0" |
| 127–128 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 129–130 | PASS-live | ProfessionGear.log 2026-10-02 23:39: "loaded affixes=10157" (thousands of rows for items that no longer exist), normal startup, no errors |
| 131 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 132–133 | NEEDS-SETUP | harness commands for Kenshi import and new game (menu flows) |
| 134 | PASS-live | `C:\KenshiTestRuns\m2\pg-06-persistence.out` (archive/test-run-2026-10-03-m2.md) 25/0 |
| 135–141 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 142–144 | PASS-live | `C:\KenshiTestRuns\scenarios\smoke.txt` 2026-10-02 23:47: Stobe + KenshiFP + PG + harness in one game (help lists fp_mode/pg_bonus/stobe_say; stobe_ping, fp_state, pg_bonus answer) |
| 145 | NEEDS-SETUP | an equippable mod hoe/sickle/pickaxe weapon on a character: the harness cannot create weapons; needs a craftable recipe (ArkWeaponPack Sickle/Pickaxe) or a fixture NPC carrying one |
| 146 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 147 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 148 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 149 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 150 | PASS-live | `C:\KenshiTestRuns\m6\pg-31.out` (archive/test-run-2026-10-03-m5.md) 12/0 (launch 3); log "loaded affixes=3 legacyIgnored=1", no roll line |
| 151–152 | NEEDS-SETUP | a frame-time or PG scan-duration metric (spawning 100/300 NPCs works; "visible hitch" otherwise needs Shay) |
| 153–154 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 155 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 156 | PENDING-auto | `tests/ingame/auto-home/pg-09-soak.txt` |
| 157 | PASS-live | `C:\KenshiTestRuns\m2\pg-01-startup-stability.out` (archive/test-run-2026-10-03-m2.md) 50/0 |
| 158 | PASS-live | `C:\KenshiTestRuns\m4\pg-02-equip-stats.out` (archive/test-run-2026-10-03-m4.md) 115/0 (83 with +400%: vanilla effective is ~31% of base for Malzin) |
| 159 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 160 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): one crafted roll for the craft saved mid-progress |
| 161–176 | NEEDS-SETUP | balance measurement driver (N repeats per skill/bonus point, CSV out) + per-profession benchmark fixtures; Phase-8 proof first (pg-08). Missing: research bench (184), robotics job (186), turret + target (192), water route (194), detection/lock/target benchmarks (195-199) |
| 177 | NEEDS-SETUP | job-path proof ran (m4 pg-08 39/0) but `building "Stone Mine"` qty is the output buffer (0 -> 0 -> 1 over 2 h): needs a production counter (units produced per game hour) or a job whose output stays put |
| 178–199 | NEEDS-SETUP | balance measurement driver (N repeats per skill/bonus point, CSV out) + per-profession benchmark fixtures; Phase-8 proof first (pg-08). Missing: research bench (184), robotics job (186), turret + target (192), water route (194), detection/lock/target benchmarks (195-199) |
| 200–210 | DEFERRED | by design: fitting and freeze after the measurement campaign (tools/analyze_balance.py) |
| 211 | PASS-live | LIVE_TEST_PROGRESS.md "Real trader-stock context - PASS" (Blamo) |
| 212 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 213 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 214–216 | PASS-live | LIVE_TEST_PROGRESS.md "Trader item transfer / purchase-style identity - LIVE PASS" |
| 217 | NEEDS-SETUP | a trader restock trigger (game restock timer or a harness restock command) |
| 218–219 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 220–227 | NEEDS-SETUP | fixtures/teleports next to several shop types (clothing/armour, tech, bar/cook, smith, doctor) + a Normal-mode pg_census per shop |
| 228 | PASS-live | `C:\KenshiTestRuns\m3\pg-05-npc-context.out` (archive/test-run-2026-10-03-m3.md) 54/0 |
| 229 | NEEDS-SETUP | an equippable mod hoe/sickle/pickaxe weapon on a character: the harness cannot create weapons; needs a craftable recipe (ArkWeaponPack Sickle/Pickaxe) or a fixture NPC carrying one |
| 230 | PASS-live | `C:\KenshiTestRuns\m4\pg-03-identity.out` (archive/test-run-2026-10-03-m4.md) 44/0 |
| 231–234 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 235–236 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 237 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 238 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 239 | PASS-live | `C:\KenshiTestRuns\m13\pg-21-shop-stock.out` 39/0 (Trader fixture, Habul\'s shop counter, recruited buyer; DLL 59EFB4B1) |
| 240 | NEEDS-SETUP | harness commands for Kenshi import and new game (menu flows) |
| 241–243 | PASS-live | LIVE_TEST_PROGRESS.md "Exact-ID harness + world-loot live results" |
| 244 | PASS-live | `C:\KenshiTestRuns\m5\pg-30.out` (archive/test-run-2026-10-03-m5.md) 21/0 (launch 2, AutoClassify off) |
| 245 | PASS-live | `C:\KenshiTestRuns\m6\pg-41.out` (archive/test-run-2026-10-03-m5.md) launch 4: 30 world-loot Rattan Hats (tier 6, chance .96 x .5 = .48) rolled 15/30 = .50 |
| 246–249 | PASS-live | `C:\KenshiTestRuns\m4\pg-07-worldloot.out` (archive/test-run-2026-10-03-m4.md) 31/0 |
| 250 | NEEDS-SETUP | a ruin/chest loot fixture (unlooted world containers) |
| 251–252 | PASS-offline | core unique/legendary tests; a live check needs a unique item instance in a fixture |
| 253 | PASS-live | `C:\KenshiTestRuns\m8\pg-11-real-craft.out` 39/0 + that launch's ProfessionGear.log (archived under `C:\KenshiTestRuns\logs\`): high-quality crafted items roll |
| 254 | NEEDS-SETUP | pg-02 ran (m2-m4) but harness `stat maxrunspeed` reads 0.0 with and without +50% Athletics (standing/paused): needs a speed readout while running or a timed walk |
| 255 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 256 | NEEDS-SETUP | swim gear in the mod stack + a water route |
| 257 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 258–271 | DEFERRED | by design (Shay): FCS profession items and selective stacking are later goals |
| 272 | PASS-live | `C:\KenshiTestRuns\m3\pg-04-backpacks.out` (archive/test-run-2026-10-03-m3.md) 69/0 |
| 273 | PASS-live | LIVE_TEST_PROGRESS.md 309 (Bolts) + 308-311; rechecked in pg-03 |
| 274 | NEEDS-SETUP | a save whose item carries a persistent ID that the sidecar holds for another base item |
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
