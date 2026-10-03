# Profession Gear Progression — in-game test status

**Updated:** 2026-10-03. One row per `TEST_PLAN.md` ID (1–329 plus 16a–16c and 30a–30i = 341 rows).
The coordinator session runs the game; the PG agent writes scenarios (`tests/ingame/`, order in
`tests/ingame/RUN_ORDER.md`) and fixes. Build for these scenarios: **ProfessionGearProgression.dll
FAA5B471** (repo HEAD at the time of writing; installed is still D667F5EE).

States: **PASS-offline** (core tests / build gates / source contracts), **PASS-live** (with evidence),
**PENDING-auto** (a scenario file exists; the harness can run it without Shay), **NEEDS-SETUP**
(missing situation/fixture/command, named in the row), **NEEDS-SHAY** (eyes/feel only),
**DEFERRED** (by design).

| State | Rows |
|---|---|
| PASS-offline | 73 |
| PASS-live | 62 |
| PENDING-auto | 107 |
| NEEDS-SETUP | 63 |
| NEEDS-SHAY | 10 |
| DEFERRED | 26 |
| total | 341 |

## Rows

| ID | State | Evidence / scenario / what is missing |
|---|---|---|
| 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16a, 16b, 16c, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 30a, 30b, 30c, 30d, 30e, 30f, 30g, 30h | PASS-offline | run_tests.bat (core), build/export/package gates; 4 as amended by 307 |
| 30i | PASS-offline | INSTALLED_MOD_GEAR_AUDIT.md + 2026-10-03 weapon-description corpus scan, turned into regressions (322) |
| 31–41 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 1 - automated startup PASS": all hooks "hooked", world loads |
| 42–43 | PENDING-auto | `tests/ingame/auto-home/pg-01-startup-stability.txt` |
| 44 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2 persistence: saves pg-phase2-equipped / pg-persist-v2 without crash |
| 45 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2: sidecar reloaded after a full Kenshi restart (318) |
| 46–48 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 2 equipped-section rerun" + "Phase 2 - equip-only + persistence core PASS" |
| 49 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 50 | NEEDS-SETUP | harness drop-to-ground + pick-up commands (storage moves exist: pg_store/pg_take, used for 52/246) |
| 51–56 | PENDING-auto | `tests/ingame/auto-home/pg-03-identity.txt` |
| 57 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 58 | PENDING-auto | `tests/ingame/auto-home/pg-03-identity.txt` |
| 59–62 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 63 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2: Square Goggles Engineering 20.7 -> 21.8 -> 20.7, Science 20.9 -> 22.0, base untouched (of 62-80 only these stats were live) |
| 64 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 65–66 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2: Square Goggles Engineering 20.7 -> 21.8 -> 20.7, Science 20.9 -> 22.0, base untouched (of 62-80 only these stats were live) |
| 67–79 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 80 | PASS-live | LIVE_TEST_PROGRESS.md Phase 2: Square Goggles Engineering 20.7 -> 21.8 -> 20.7, Science 20.9 -> 22.0, base untouched (of 62-80 only these stats were live) |
| 81–82 | PENDING-auto | `tests/ingame/auto-home/pg-08-job-path.txt` (177: first points only; the full curve is NEEDS-SETUP) |
| 83 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 84 | NEEDS-SETUP | harness `stat` has no melee attack/defence/dodge/toughness names (code passes unmapped stats through unchanged) |
| 85–86 | PENDING-auto | `tests/ingame/crafting-base/pg-10-craftfinish.txt` |
| 87 | PENDING-auto | `tests/ingame/crafting-base/pg-11-real-craft.txt` (90 also in pg-10) |
| 88 | PENDING-auto | `tests/ingame/crafting-base/pg-10-craftfinish.txt` |
| 89 | NEEDS-SETUP | a way to force a critical-success craft (or many weapon crafts with a known crit chance) |
| 90 | PENDING-auto | `tests/ingame/crafting-base/pg-11-real-craft.txt` (90 also in pg-10) |
| 91–92 | PENDING-auto | `tests/ingame/crafting-base/pg-10-craftfinish.txt` |
| 93 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 94 | PENDING-auto | `tests/ingame/auto-home/pg-05-npc-context.txt` |
| 95–97 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 98–100 | PENDING-auto | `tests/ingame/auto-home/pg-05-npc-context.txt` |
| 101 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 102–104 | PASS-offline | core rate tests over 4,000 instances (slave/poor x0.2, matching specialist x1.35); 103 needed fix 2b14a4c; live data for 102 in config/pg-40 |
| 105 | PENDING-auto | `tests/ingame/auto-home/pg-05-npc-context.txt` |
| 106 | NEEDS-SETUP | a reliable stream-out/in trigger for one NPC (unload command, or a fixture + teleport distance known to unload) |
| 107 | PENDING-auto | `tests/ingame/auto-home/pg-05-npc-context.txt` |
| 108 | NEEDS-SETUP | a named/unique NPC (isUnique) with eligible gear in a fixture, or a spawnable unique template |
| 109–116 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 117 | PASS-live | LIVE_TEST_PROGRESS.md "Generic backpack live weight control - PASS" (rechecked in pg-04) |
| 118–119 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 120 | NEEDS-SETUP | a weight readout for a pack that is nested or not owned by a character |
| 121–122 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 123–124 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 (restart) |
| 125 | PASS-live | LIVE_TEST_PROGRESS.md Phase 1 live scan: no sidecar at the first launch, created safely |
| 126 | PENDING-auto | `tests/ingame/config/pg-40-normal-distribution.txt` (own launch: -Mode NormalVerbose -Sidecar Empty) |
| 127–128 | PENDING-auto | `tests/ingame/config/pg-31-disabled-sidecar.txt` (own launch: -Mode Disabled -Sidecar StartupTest) |
| 129–130 | PASS-live | ProfessionGear.log 2026-10-02 23:39: "loaded affixes=10157" (thousands of rows for items that no longer exist), normal startup, no errors |
| 131 | PENDING-auto | `tests/ingame/auto-home/pg-06-persistence.txt` |
| 132–133 | NEEDS-SETUP | harness commands for Kenshi import and new game (menu flows) |
| 134 | PENDING-auto | `tests/ingame/auto-home/pg-06-persistence.txt` |
| 135–141 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 142–144 | PASS-live | `C:\KenshiTestRuns\scenarios\smoke.txt` 2026-10-02 23:47: Stobe + KenshiFP + PG + harness in one game (help lists fp_mode/pg_bonus/stobe_say; stobe_ping, fp_state, pg_bonus answer) |
| 145 | NEEDS-SETUP | an equippable mod hoe/sickle/pickaxe weapon on a character: the harness cannot create weapons; needs a craftable recipe (ArkWeaponPack Sickle/Pickaxe) or a fixture NPC carrying one |
| 146 | PENDING-auto | `tests/ingame/auto-home/pg-03-identity.txt` |
| 147 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 148 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 149 | PENDING-auto | `tests/ingame/config/pg-30-autoclassify-off.txt` (own launch: -Mode AutoClassifyOff) |
| 150 | PENDING-auto | `tests/ingame/config/pg-31-disabled-sidecar.txt` (own launch: -Mode Disabled -Sidecar StartupTest) |
| 151–152 | NEEDS-SETUP | a frame-time or PG scan-duration metric (spawning 100/300 NPCs works; "visible hitch" otherwise needs Shay) |
| 153–154 | PENDING-auto | `tests/ingame/auto-home/pg-01-startup-stability.txt` |
| 155 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 156 | PENDING-auto | `tests/ingame/auto-home/pg-09-soak.txt` |
| 157 | PENDING-auto | `tests/ingame/auto-home/pg-01-startup-stability.txt` |
| 158 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 159 | PENDING-auto | `tests/ingame/auto-home/pg-03-identity.txt` |
| 160 | PENDING-auto | `tests/ingame/crafting-base/pg-11-real-craft.txt` (90 also in pg-10) |
| 161–176 | NEEDS-SETUP | balance measurement driver (N repeats per skill/bonus point, CSV out) + per-profession benchmark fixtures; Phase-8 proof first (pg-08). Missing: research bench (184), robotics job (186), turret + target (192), water route (194), detection/lock/target benchmarks (195-199) |
| 177 | PENDING-auto | `tests/ingame/auto-home/pg-08-job-path.txt` (177: first points only; the full curve is NEEDS-SETUP) |
| 178–199 | NEEDS-SETUP | balance measurement driver (N repeats per skill/bonus point, CSV out) + per-profession benchmark fixtures; Phase-8 proof first (pg-08). Missing: research bench (184), robotics job (186), turret + target (192), water route (194), detection/lock/target benchmarks (195-199) |
| 200–210 | DEFERRED | by design: fitting and freeze after the measurement campaign (tools/analyze_balance.py) |
| 211 | PASS-live | LIVE_TEST_PROGRESS.md "Real trader-stock context - PASS" (Blamo) |
| 212 | PENDING-auto | `tests/ingame/trader/pg-21-shop-stock.txt` (Habul's shop; 219/237 were PARTIAL; fixes d3ac3dc + 1da2eb0 scan shop storage within 60) |
| 213 | PENDING-auto | `tests/ingame/auto-home/pg-07-worldloot.txt` |
| 214–216 | PASS-live | LIVE_TEST_PROGRESS.md "Trader item transfer / purchase-style identity - LIVE PASS" |
| 217 | NEEDS-SETUP | a trader restock trigger (game restock timer or a harness restock command) |
| 218–219 | PENDING-auto | `tests/ingame/trader/pg-21-shop-stock.txt` (Habul's shop; 219/237 were PARTIAL; fixes d3ac3dc + 1da2eb0 scan shop storage within 60) |
| 220–227 | NEEDS-SETUP | fixtures/teleports next to several shop types (clothing/armour, tech, bar/cook, smith, doctor) + a Normal-mode pg_census per shop |
| 228 | PENDING-auto | `tests/ingame/auto-home/pg-05-npc-context.txt` |
| 229 | NEEDS-SETUP | an equippable mod hoe/sickle/pickaxe weapon on a character: the harness cannot create weapons; needs a craftable recipe (ArkWeaponPack Sickle/Pickaxe) or a fixture NPC carrying one |
| 230 | PENDING-auto | `tests/ingame/auto-home/pg-03-identity.txt` |
| 231–234 | PASS-offline | classifier corpus tests (ordinary/legendary weapons, profession-named weapons); 59 rechecked live in pg-02 |
| 235–236 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 237 | PENDING-auto | `tests/ingame/trader/pg-21-shop-stock.txt` (Habul's shop; 219/237 were PARTIAL; fixes d3ac3dc + 1da2eb0 scan shop storage within 60) |
| 238 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 239 | PENDING-auto | `tests/ingame/trader/pg-21-shop-stock.txt` (Habul's shop; 219/237 were PARTIAL; fixes d3ac3dc + 1da2eb0 scan shop storage within 60) |
| 240 | NEEDS-SETUP | harness commands for Kenshi import and new game (menu flows) |
| 241–243 | PASS-live | LIVE_TEST_PROGRESS.md "Exact-ID harness + world-loot live results" |
| 244 | PENDING-auto | `tests/ingame/config/pg-30-autoclassify-off.txt` (own launch: -Mode AutoClassifyOff) |
| 245 | PENDING-auto | `tests/ingame/config/pg-41-normal-worldloot.txt` (same launch as pg-40) |
| 246–249 | PENDING-auto | `tests/ingame/auto-home/pg-07-worldloot.txt` |
| 250 | NEEDS-SETUP | a ruin/chest loot fixture (unlooted world containers) |
| 251–252 | PASS-offline | core unique/legendary tests; a live check needs a unique item instance in a fixture |
| 253 | PENDING-auto | `tests/ingame/crafting-base/pg-11-real-craft.txt` (90 also in pg-10) |
| 254 | PENDING-auto | `tests/ingame/auto-home/pg-02-equip-stats.txt` |
| 255 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 256 | NEEDS-SETUP | swim gear in the mod stack + a water route |
| 257 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 258–271 | DEFERRED | by design (Shay): FCS profession items and selective stacking are later goals |
| 272 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 273 | PASS-live | LIVE_TEST_PROGRESS.md 309 (Bolts) + 308-311; rechecked in pg-03 |
| 274 | NEEDS-SETUP | a save whose item carries a persistent ID that the sidecar holds for another base item |
| 275–276 | PASS-live | implicit in every live run (per-character bonus with several characters, inventory callbacks under heavy moves, sidecar atomic replace); the failure path of 279 is offline only |
| 277 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 278 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 279 | PASS-live | implicit in every live run (per-character bonus with several characters, inventory callbacks under heavy moves, sidecar atomic replace); the failure path of 279 is offline only |
| 280 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 281–282 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 283–288 | PASS-offline | core config clamp tests, source contract (double start), classifier corpus, package/installer verifiers |
| 289 | PASS-live | every ProfessionGear.log: version + normalized config + rule counts before the hook lines |
| 290 | DEFERRED | superseded: installs go through install-dll.ps1, no DLL backups (git is the history) |
| 291–292 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 293–297 | PASS-offline | classifier semantic-boundary and profession-pack tests |
| 298 | PENDING-auto | `tests/ingame/config/pg-30-autoclassify-off.txt` (own launch: -Mode AutoClassifyOff) |
| 299 | PASS-live | LIVE_TEST_PROGRESS.md generic pack weight half; Athletics half rechecked in pg-04 |
| 300 | PASS-live | verbose roll lines with source/key/baseId/name/tier/tags/affixes in every live log; rechecked in pg-02 |
| 301–305 | PASS-offline | classifier semantic-boundary and profession-pack tests |
| 306 | PENDING-auto | `tests/ingame/crafting-base/pg-10-craftfinish.txt` |
| 307 | PASS-live | LIVE_TEST_PROGRESS.md RE_Kenshi loader contract |
| 308–311 | PASS-live | LIVE_TEST_PROGRESS.md "Live non-equippable regression" + "Exact-ID harness" (309) |
| 312–313 | PASS-live | LIVE_TEST_PROGRESS.md "Phase 2 equipped-section rerun" |
| 314–318 | PASS-live | LIVE_TEST_PROGRESS.md "Persistence v2 live verification" + Phase 2 restart |
| 319 | PASS-offline | v2 loader ignores pre-v2 rows; live recheck in config/pg-31 (legacyIgnored=1) |
| 320 | PASS-live | LIVE_TEST_PROGRESS.md "Contextual generic coherence fix - LIVE PASS" + "Additional NPC role-context live passes" |
| 321 | PENDING-auto | `tests/ingame/config/pg-30-autoclassify-off.txt` (own launch: -Mode AutoClassifyOff) |
| 322 | PASS-offline | core regressions added 2026-10-03 (bd6643c, 2b14a4c) |
| 323–324 | PENDING-auto | `tests/ingame/trader/pg-21-shop-stock.txt` (Habul's shop; 219/237 were PARTIAL; fixes d3ac3dc + 1da2eb0 scan shop storage within 60) |
| 325 | PASS-offline | core regressions added 2026-10-03 (bd6643c, 2b14a4c) |
| 326 | PENDING-auto | `tests/ingame/config/pg-31-disabled-sidecar.txt` (own launch: -Mode Disabled -Sidecar StartupTest) |
| 327 | PENDING-auto | `tests/ingame/config/pg-30-autoclassify-off.txt` (own launch: -Mode AutoClassifyOff) |
| 328 | PENDING-auto | `tests/ingame/auto-home/pg-04-backpacks.txt` (needs -Rules InGameTest) |
| 329 | PENDING-auto | `tests/ingame/crafting-base/pg-10-craftfinish.txt` (fix FAA5B471: launch 1 showed the crafted roll landing on the wrong instance) |

## Notes

- PASS-live evidence predates the 2026-10-03 fixes (shop storage scan, purchase hook, weapon names,
  poor-NPC wealth, inactive stale records, CRLF sidecar). Those touch rows 101-104, 211-219, 237, 298,
  321-328; the PENDING-auto scenarios recheck the live ones (pg-03, pg-04, pg-20, pg-30).
- Scenario reply formats come from the current harness and PG commands (`pg_info`, `pg_check`,
  `pg_bonus` with `match=1`, `pg_pack` ratios, `pg_shop`, `pg_building`, `pg_take`, `pg_store`,
  `pg_loot`, `pg_census`). Unknowns that may need a one-line scenario fix on the first run are
  noted in each file's header (fixture item names, building names, auto-equip of given items).
