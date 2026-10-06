# Profession Gear Progression — in-game test status

**Updated:** 2026-10-06 (doc cleanup). One row per open `TEST_PLAN.md` ID. All automated rows are PASS (offline or
live) and were removed from the table below; per-row evidence of every removed PASS row: `git show 71dbe14:INGAME_STATUS.md`.
Removed PASS rows: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16a, 16b, 16c, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 30a, 30b, 30c, 30d, 30e, 30f, 30g, 30h, 30i, 31–41, 42–43, 44, 45, 46–48, 49, 50, 51–56, 57, 58, 59–62, 63, 64, 65–66, 67–79, 80, 81–82, 83, 84, 85–86, 87, 88, 89, 90–92, 93, 94, 95–97, 98–100, 101, 102, 103–104, 105, 106, 107, 108, 109–116, 117, 118–119, 120, 121–122, 123–124, 125, 126, 127–128, 129–130, 131, 133, 134, 142–144, 145, 146, 147, 148, 149, 150, 151–152, 153–154, 156, 157, 158, 159, 160, 161–176, 177, 178–183, 184, 185, 186–187, 188, 189, 190, 191, 192, 193–198, 199, 200–210, 211, 212, 213, 214–216, 217, 218–219, 228, 229, 230, 231–234, 235–236, 237, 239, 240, 241–243, 244, 245, 246–249, 250, 251–252, 253, 255, 256, 257, 272, 273, 274, 275–276, 278, 279, 280, 281–282, 283–288, 289, 291, 292, 293–297, 298, 299, 300, 301–305, 306, 307, 308–311, 312–313, 314–318, 319, 320, 321, 322, 323–324, 325, 326, 327, 328, 329, 330, 331.
Builds: ProfessionGearProgression.dll BAFB8C31 (Normal) on both rigs.

States: **NEEDS-SHAY** (eyes/feel only), **DEFERRED** (by design), **CLOSED** (not ours).

## Balance state (2026-10-06)

Rule (RUN_ORDER.md "Gate"): no balance matrix runs before its profession's gate passes (g1 skill 25 no gear, g2 skill 25
+ own +50%, g3 skill 90 no gear, g4 drift bracket; `tools/balance_driver.py gate|csv|gatecheck`, native evidence per
point, `stat <worker> <stat> ~ wounds=1.0000` before every measurement). "Gear applies" (mechanical) is tracked apart from
"balance curve accepted". Every profession's gate and matrix ran (m28-m47); Shay decisions D1-D8 were applied
(`MASTER_TEST_PLAN.md` section 2, run log `archive/test-run-2026-10-05-m41.md`): farming gear ~0.5/pt (206 PASS), medic gear
raises kit quality (191 PASS), Perception back on gear (199 ranged gate PASS m47), crossbow no Labouring leak, turrets
accepted, 132 closed (vanilla crash). Open: D4 athletics feel (Shay) and the whole-picture balance decision.

Still-valid findings:
- Lockpicking: KenshiLib's `Character::getLockpickChance` is a dead stub in the running exe (RVA 0x884DC0, returns 0.0);
  the real chance is the DoorLock fn (RVA 0x297D80): 0.9 if lock level <= skill, else 0.9 / 2^((level - skill) / 10),
  skill = getStat(LOCKPICKING) - 10 when caged. Gear works through the getStat hook. **To do:** remove the no-op
  `HookLockpick` (src/ProfessionGearPlugin.cpp). A lock with level <= skill reads a flat 0.9: use a higher-level lock.
- Assassination, stealth, swimming formulas read raw CharStats members: the hooked value moves only with FormulaScaling.

## Rows

| ID | State | Evidence / scenario / what is missing |
|---|---|---|
| 132 | CLOSED (vanilla bug, Shay D7 2026-10-05) | Import into a fresh new game crashes Kenshi (kenshi_x64+0x94d6db AV read 0x270) with and without the PG DLL and on the 4080 without Stobe/KenshiFP (dump `C:\KenshiTestRuns\crash-m37-i4-nopg\`): a game bug, not PG |
| 135–141 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 155 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 238 | NEEDS-SHAY | feel: repeated tooltip opening / shop-open stall |
| 220–227 | PASS-live (measured; finding for Shay) | `C:\KenshiTestRuns\m18-4080\pg-60-squin-shops-4080.out` 53/15 + `pg-63-squin-shopkeepers-4080.out` 22/0 (4080, Normal chances, empty sidecar; the 15 FAILs are trader slots 8-12: Squin has 7 traders within 300 m). Squin shops: 7 shopkeepers, eligible stock only at Trader Tentpeg (7) and Double (3, armour), 0 at the barman and 3 apothecaries; census npc 210-247 records, 112-113 rolled: Athletics 53-57, Perception 47-54, Medic 22-27 (222 measurable), Engineering 2 (223), Robotics 1 (225), Stealth/Lockpicking/Thievery 2-3; traders 97 records, 35 rolled. Farming (220), Labouring (221), Science (224), Cooking (226), Smithing (227): 0 in Squin (also ~0 in the Trader town, m6). Production gear practically never appears in shops: a rarity/source decision for Shay (no balance change until the balance data is complete) |
| 254 | PASS-live (mechanical); feel = Shay (D4) | run speed: pg-74-walktime 34/0 (m36) max 81.8 -> 113.1 u/s at +50% Athletics. Acceleration (Shay D4, PG f212cbc: updateVelocity clamp + HavokCharacter::update braking curve scaled): pg-94 m43 batches F/G/H on screen 12/12: t50 0.300 vs 0.400 s (0.74-0.75) PASS, top speed unchanged PASS; t90 and stop distance (move order or halt) unchanged: those come from the path slow-down / order delay / halt routine, not the acceleration value (`C:\KenshiTestRuns\m42\out-g`, `out-h`) |
| 258–271 | DEFERRED | by design (Shay): FCS profession items and selective stacking are later goals |
| 277 | NEEDS-SHAY | tooltip text is not readable by the harness (ui lists widgets, not hover tooltips) |
| 290 | DEFERRED | superseded: installs go through install-dll.ps1, no DLL backups (git is the history) |

## Notes

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
