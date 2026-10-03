# Profession Gear Progression

Standalone RE_Kenshi plugin that adds contextual profession-equipment progression to Kenshi.

## What it does

- Classifies existing vanilla and mod-added equipment by profession context.
- Rolls per-instance profession affixes using item quality/tier.
- Applies profession bonuses at `CharStats::getStat` without modifying base skills or XP.
- Caches equipped profession aggregates per character; inventory add/remove/update callbacks refresh the cache, with the 1-second loaded-character sweep retained as discovery/safety fallback.
- Gives player-crafted items their roll at `CraftingBuilding::addFinishedCraftItem`.
- Uses the owning NPC's strongest profession skills as role context.
- Heavily suppresses special gear rolls for slaves/very low-tier NPCs.
- Persists item-instance rolls in `profession_gear_affixes.tsv`.
- Adds profession lines to tooltips for supported equipment classes.
- Supports exact item overrides and exclusions in `ProfessionGear.rules`.
- Adds category-specific weight specialization for ore, crop, construction, medical, trade and tech packs.

## Supported profession stats

Labouring, Science, Engineering, Robotics, Weapon Smithing, Armour Smithing,
Crossbow Smithing, Medic, Turrets, Farming, Cooking, Athletics, Swimming,
Perception, Stealth, Assassination, Lockpicking and Thievery.

## Build

Use the same VS2010/Kenshi SDK toolchain as STOBE:

```
build_portable.bat
run_tests.bat
verify_offline.bat
```

Output:
`out\ProfessionGearProgression.dll`

For the complete pre-install gate, run `verify_ready.bat`. It runs core tests, builds the DLL, verifies all required SDK symbols/exports, creates and verifies the installable package, and confirms ProfessionGear is not already present in the known Kenshi mod paths.

The portable build uses:
- `C:\StobeBuildTools` VS2010/SDK 7.1 toolchain
- `C:\StobeBuild\sdk`
- `C:\StobeBuild\boost`

## Packaging

Run `package.bat` after a successful build. It creates:

`out\package\ProfessionGearProgression\`

For live testing, follow **`LIVE_TEST_RUNBOOK.md`**. `TEST_PLAN.md` is the 306-row technical master plan; you do not need to manually score every AUTO row.

Nothing in the build or package scripts copies files into Kenshi. The guarded `install_test_build.ps1` script is provided for the first controlled live test, but it refuses to install unless explicitly invoked with `-Install`.

## Runtime files

- `ProfessionGear.ini`: global tuning.
- `ProfessionGear.rules`: exact compatibility overrides/exclusions.
- `profession_gear_affixes.tsv`: generated per-instance persistence sidecar.
- `ProfessionGear.log`: startup/hook diagnostics.

## Rules syntax

```
exclude|some_mod_item_string_id
tag|some_mod_hoe_string_id|TOOL_FARMING
tag|some_mod_research_outfit|BODY_RESEARCH,HEAD_RESEARCH
tag|some_mod_ore_pack|PACK_ORE
```

Explicit rules beat automatic name/category classification.

## Important implementation rule

The plugin changes effective stat reads only. It does not write equipment bonuses into
the stored base profession skill values. Unequipping the item therefore removes the
bonus without changing earned XP.

## Gear classification

`VANILLA_GEAR_CLASSIFICATION.md` documents the reviewed vanilla equipment catalogue and the revised hybrid classifier. Automatic support is not ID-only: exact rules are strongest, but native item type/slot, descriptions, semantic names, and source/NPC context are combined so unknown third-party gear can still classify when evidence is strong. Weak/contradictory cases receive no profession affix rather than a guess.

Affix-count progression is now tiered: low tiers (0–2) max 1 stat, medium tiers (3–4) up to 2, top tiers (5–6) up to 3 coherent stats. Utility classification also covers travel/Athletics, Swimming, stealth/assassination/thief gear, generic backpacks, hauling packs, and specialist cargo packs. Strong profession/utility semantics take precedence over generic fallback.

## Design audit

`DESIGN_IMPLEMENTATION_AUDIT.md` maps Shay's full design/context to the current implementation, live-proof requirements, balance work, and explicitly deferred roadmap items.

## Roadmap

`ROADMAP.md` records approved later scope: broader native utility effects where they make sense, profession-themed item generation using reused Kenshi assets, normal vendor/loot/NPC/crafting distribution, and selective specialist-pack stacking if it can be implemented safely.

## Balance calibration

The current affix percentages are provisional. `BALANCE_TEST_PLAN.md` defines a 50-test live calibration matrix (tests 161–210) that measures real Kenshi job throughput across skill bands and synthetic bonus levels. `tools/analyze_balance.py` converts captured CSV runs into throughput curves and candidate effective-skill bonus ranges. Tests 211–240 cover trader-stock generation, equip-only behavior, profession availability/frequency, installed-mod distribution, unique-weapon protection, and other live assumptions. Tests 241–250 cover exploration/world-loot generation, reduced world-loot rarity, player-profession independence, chest discovery timing, and ruin-loot distribution. Tests 251–272 cover unique-item protection, broader utility effects, generated profession-themed item records/distribution, and specialist-pack stacking feasibility. Tests 273–300 cover pre-install runtime hardening, package/install safety, semantic-boundary regressions, utility-role weapons, equip-only specialist packs, persistence defenses, and verbose diagnostics. Tests 301–305 cover automatic specialist classification for profession-named backpacks/satchels. Final tier values should be frozen only after those in-game measurements are complete.

## Current validation boundary

The core logic and final DLL are built and tested without installing the mod.
All game-facing hooks require the in-game test plan before release because Kenshi ABI,
item identity persistence, tooltip dispatch and job-stat call paths can only be proven
inside the live game.
