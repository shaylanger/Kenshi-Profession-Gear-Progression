# Profession Gear Progression — Roadmap

This file records approved future scope that is **part of the mod vision** but does not need to block the first runtime-compatible release.

## Goal 1 — Broad utility progression

Built and verified in game (2026-10-06): Athletics/run speed (+ acceleration hook), Swimming, Perception, Stealth,
Assassination, Lockpicking/Thievery, hauling/specialist backpack weight (rows 254-257, 272, balance gates m28-m47).
Principle kept for new effects: prefer existing Kenshi mechanics, no parallel stats, only when the item name/type fits.

## Goal 2 — Profession-themed item generation/content layer

Add a later optional FCS/content layer that creates more discoverable profession gear by reusing existing Kenshi/mod-compatible models and item bases with new profession-themed records/names.

The goal is **not** to require custom art. Reuse existing visual assets and create coherent variants.

Examples:
- Farmer's Sword
- Farmer's Sickle / Pitchfork
- Miner's Pickaxe
- Miner's Backpack
- Engineer's Goggles
- Engineer's Workwear
- Field Medic Coat / Mask / Pack
- Researcher's Goggles / Coat / Satchel
- Cook's Apron / Knife
- Smith's Gloves / Apron / Hammer
- Traveler's Sandals
- Scout Boots
- Load-Bearing Harness
- Ore / Crop / Construction / Medical / Tech packs

These new records should:
- use normal Kenshi equipment models/assets,
- have normal Kenshi slots/types,
- be recognized naturally by the runtime classifier from their names/descriptions,
- appear in appropriate vendors/loot/NPC equipment/crafting,
- remain compatible with the same per-instance affix system,
- avoid replacing or editing third-party records when a new standalone variant is sufficient.

This layer exists to improve discoverability and broaden categories that vanilla/UWE/GenMod do not represent strongly enough.

## Goal 3 — Item-generation/distribution support

ProfessionGear should eventually support deliberate generation/distribution of profession-themed item variants rather than relying only on passive classification.

Desired distribution channels:
- armour/weapon/travel/medical/tech vendors,
- ruin/chest loot,
- profession NPCs,
- faction-appropriate NPC equipment,
- player crafting,
- blueprints/research where appropriate.

Examples:
- armour shop occasionally stocks Engineer's Workwear,
- travel shop can stock Traveler's Sandals,
- medical vendor can stock Field Medic gear,
- farming settlement NPC may carry Farmer's Sword or Sickle,
- ruin can contain old Engineer/Research gear,
- player can craft a Miner's Backpack after learning its blueprint.

This should use normal Kenshi content systems where possible. Runtime affixes remain per-instance and separate from the FCS base record.

## Goal 4 — Specialist backpack stacking

Current v1 implementation provides category-specific weight reduction.

Desired later behavior:
- specialist pack should also stack its intended cargo more efficiently where feasible.

Examples:
- ore pack -> ore stacking
- crop pack -> crop stacking
- medical pack -> medical supplies
- construction pack -> building materials
- tech pack -> research items

Current runtime API appears to expose section-wide stacking rather than per-item selective stacking. Therefore:
- do not implement unsafe global stacking that benefits unrelated items,
- prefer FCS-level specialist pack restrictions/stacking if that can achieve the behavior cleanly,
- keep this as a roadmap goal if selective stacking remains technically awkward.

## Goal 5 — Preserve authored unique gear

Built and verified (rows 251-253): `Item::isUnique`, Meitou/Cross and legendary markers are never augmented.

## Release philosophy

V1 should prioritize:
1. correct contextual classification,
2. existing/mod-added item support,
3. NPC/vendor/exploration/crafting rolls,
4. equip-only real effects,
5. persistence,
6. balanced progression,
7. stability/compatibility.

The optional content layer and harder utility/stacking systems can follow once the runtime foundation is proven in-game.
