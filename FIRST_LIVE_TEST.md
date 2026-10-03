# Profession Gear Progression — First Live Test

**Player-facing canonical instructions:** `LIVE_TEST_RUNBOOK.md`.

This file is the shorter technical phase map. The first controlled Kenshi validation begins only after Shay explicitly authorizes installation.

## Safety rules

- Do not use Shay's main save.
- Use a copied/fixture save or a new test game.
- Archive `ProfessionGear.log` and `profession_gear_affixes.tsv` between major phases.
- Start with ProfessionGear + RE_Kenshi/KenshiLib only if practical, then test with STOBE/KenshiFP and the normal mod list.
- A failed live test gets a regression row before the fix is considered complete.

## Install

The repository contains `install_test_build.ps1`.

It deliberately refuses to install unless called with:

`-Install`

Default Kenshi root:
`D:\Steam\steamapps\common\Kenshi`

Example:
`powershell -ExecutionPolicy Bypass -File .\install_test_build.ps1 -Install`

The script backs up any existing ProfessionGearProgression folder before copying the package. It does not change the launcher load order.

## Optional forced-roll smoke config

For short functional smoke tests, `tests\fixtures\ProfessionGear.forced-test.ini` sets all eligible source chances to succeed and enables verbose generation logging. Copy it over the installed `ProfessionGear.ini` only for the controlled fixture run, then restore the normal packaged config before distribution/balance testing.

## Phase 1 — startup / ABI smoke

Run tests 31–45 first.

Success gate:
- Kenshi reaches a loaded world.
- ProfessionGear log reports startup and all hooks.
- no crash on pause/unpause/50x speed,
- save and normal exit work,
- sidecar writes atomically.

Stop immediately on hook-chain crash before testing gameplay effects.

## Phase 2 — identity / equip-only smoke

Prioritize:
- 46–53
- 62–84
- 123–134
- 214–219
- 241–249

Critical proof:
- carrying eligible gear gives zero bonus,
- equipping activates exactly one bonus,
- unequipping returns to baseline,
- same item retains its roll through transfer/drop/save/load,
- item handle reuse does not attach another item's roll,
- player pickup does not bias world loot toward the player's profession.

## Phase 3 — real job-path proof

Before balancing values, prove that the hooked effective stats actually change gameplay.

Run:
- Farming
- Labouring
- Science
- Engineering
- Cooking
- Medic
- smithing

Do not begin final balance fitting until job throughput moves in the expected direction.

## Phase 4 — source/distribution proof

Run:
- NPC role generation
- trader inventory ownership/restock
- vendor profession distribution
- ruin/chest world loot
- player crafting
- UWE/GenMod classifier corpus

This phase determines whether categories are merely technically possible or actually discoverable at useful rates.

## Phase 5 — utility / backpack proof

Validate:
- Athletics/travel gear
- Swimming gear
- Stealth/Assassination/Thief gear
- generic backpack Athletics rolls
- hauling pack all-cargo reduction
- ore/crop/construction/medical/tech/trade specialist reductions
- vanilla backpack multipliers and stacking remain intact

## Phase 6 — coexistence

Run with:
1. ProfessionGear + STOBE
2. ProfessionGear + KenshiFP
3. ProfessionGear + STOBE + KenshiFP
4. normal installed mod stack including UWE/GenMod

Check hook order, tooltips, saves, FPS/stall behavior, and inventories.

## Phase 7 — balance

Only after the prior phases pass:
- run tests 161–210,
- capture CSV,
- run `tools\analyze_balance.py`,
- replace placeholder tier ranges with measured profession-specific values if needed.

## Release blockers

Do not call the mod release-ready until:
- item identity survives save/load/streaming/import policy is defined,
- important jobs use the effective stat path,
- shop stock and world loot generate correctly,
- tooltips do not duplicate,
- specialist pack weight math is correct live,
- no coexistence crash with STOBE/KenshiFP,
- balance calibration is complete.
