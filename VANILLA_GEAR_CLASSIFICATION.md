# Vanilla Gear Classification Review

## Why this file exists

The first classifier used conceptual examples such as lab coats, chef gear, hoes and profession tools. Some of those are valid targets for new content or third-party mods, but they are not faithful descriptions of vanilla Kenshi's actual equipment catalogue.

This review grounds automatic classification in real vanilla equipment families and separates:

1. real vanilla equipment that can be classified confidently,
2. real vanilla equipment that can only receive broad utility roles safely,
3. profession-specific gear that should come from new FCS content or mod-added items,
4. semantic fallback rules for third-party mods.

## Real vanilla equipment families reviewed

### Headgear

Vanilla headgear includes:

Ashland Hat; Ashlander Stormgoggles; Basket Hat; Bandana; Black Bandana; Blackened Chainmail Tagelmust; Bucket Zukin; Cap; Crab Helmet; Dyed Turban; Flared Helmet; Fog Mask; Hachigane; Iron Hat; Karuta Zukin; Kusari Zukin; Mask Type III; Masked Helmet; Ninja Mask; Paladin's Heavy Hachigane; Police Helmet; Rattan Hat; Red Bandana; Rusty Chainmail Tagelmust; Samurai Helmet; Sandogasa; Side-Angle Hachigane; Skeleton Mask; Spiked Helmet; Square Goggles; Straw Hat; Swamp Ninja Mask; Tagelmust; Tin Can; Tricorn Hat; Turban; Visored Helmet; Wool Hat; plus unused Ancient Samurai Helmet.

Important real native clues:
- Ashlander Stormgoggles and Square Goggles already give Perception.
- Straw Hat and Sandogasa already give Perception.
- Swamp Ninja Mask already has a Stealth effect.
- Heavy helmets often penalize perception/combat utility.

### Shirts

Black Cloth Shirt; Cloth Shirt; Dyed Turtleneck; Martial Artist Bindings; Turtleneck; White Vest; Dark Leather Shirt; Leather Hive Vest; Leather Shirt; Leather Turtleneck; Leather Vest; Blackened Chain Hive Shirt; Blackened Chain Shirt; Blackened Chainmail; Chain Shirt; Chainmail; Hiver Chain Shirt; Rusted Hive Shirt; Rusty Chain Shirt; Rusty Chainmail.

These are mostly generic under-layers. They should not automatically gain profession stats from slot alone.

### Body armour / outer clothing

Clothing:
Black Rag Shirt; Dyed Rag Shirt; Gi; Hive Prisoner Shackles; Holy Servant Rags; Human Skin; Human Skin Suit; Monk Robe; Ninja Gi; Noble's Robe; Rag Shirt.

Light:
Assassin's Rags; Drifters Leather Jacket; Dustcoat; Dyed Robes; Heart Protector; Longcoat; Ninja Rags; Sleeveless Dustcoat; Sleeveless Longcoat; Swamp Ninja Rags; Trader's Leathers.

Medium:
Armoured Rags; Black Plate Jacket; Conscript Leather Armour; Hack Stopper Jacket; Mercenary Leather Armour; Plate Jacket; White Plate Jacket.

Heavy:
Ancient Samurai Armour; Crab Armour; Empire Samurai Armour; Holy Chest Plate; Mercenary Plate; Samurai Armour; Unholy Chest Plate.

Several already encode clear combat/stealth/trade semantics. Generic body armour should not receive Farming, Science, Cooking, etc. automatically.

### Legwear

Clothing:
Cargopants; Cargopants (colored); Dyed Trousers; Gi Pants; Halfpants (colored); Halfpants (ragged); Monk Pants; Ninja Pants; Noble's Trousers; Rag Loincloth; Rag Loincloth (Dyed); Stout Hessian; Stout Hessian Uniform; Worn-out Shorts.

Light:
Cargopants (padded); Cargopants (reinforced); Cargopants (sneaky chain); Drifters Leather Pants; Halfpants (padded); Halfpants (reinforced); Halfpants (sneaky chain); Samurai Clothpants.

Medium:
Armoured Rag Skirt; Hack Stopper Pants; Plated Drifter's Leather Pants.

Heavy:
Ancient Samurai Legplates; Crab Trousers; Samurai Legplates.

Again, slot alone is not enough to infer a profession.

### Footwear

Faulty Prisoner Shackles; Prisoner Shackles; Wooden Sandals; Drifter's Boots; Plated Longboots; Crab Shoes; Samurai Boots; unused Ancient Samurai Boots.

Wooden Sandals are a strong real vanilla example because they already improve Athletics and Combat Speed. Drifter's Boots are a plausible travel/general-utility item. Heavy boots are not good candidates for profession bonuses by default.

### Backpacks

Small Thieves Backpack; Thieves Backpack; Small Backpack; Medium Backpack; Scavenger's Basket; Large Backpack; Trader's Wooden Backpack; Wooden Backpack; Bull Backpack; Garru Backpack. Other inaccessible/vendor variants also exist.

Backpacks already expose strong semantics through:
- grid size,
- weight,
- encumbrance reduction,
- stack minimum/multiplier,
- combat skill penalty,
- speed,
- stealth,
- dodge.

These native properties are more reliable than the display name for determining whether a new modded backpack is generic hauling, stealth-oriented, trade-oriented or animal cargo.

### Melee weapons

Vanilla melee classes are:
- Blunt
- Hackers
- Heavy Weapons
- Katanas
- Polearms
- Sabres

Real weapon examples include Heavy Jitte, Iron Club, Iron Stick, Jitte, Mercenary Club, Spiked Club, Combat Cleaver, Flesh Cleaver, Long Cleaver, Moon Cleaver, Short-Cleaver, Paladin's Cross, Falling Sun, Fragment Axe, Plank, Katana, Guardless Katana, Ninja Blade, Nodachi, Topper, Wakizashi, Heavy Polearm, Naginata, Naginata Katana, Polearm, Desert Sabre, Foreign Sabre, Holed Sabre, Horse Chopper, Longsword and Ringed Sabre.

Combat weapons should remain combat-first. They must not receive Farming/Cooking/Science merely because they occupy a weapon slot.

### Crossbows

Junkbow; Tooth Pick; Ranger; Spring Bat; Oldworld Bow MkI; Oldworld Bow MkII; Eagle's Cross.

Crossbows are combat gear and should remain combat/perception/ranged-oriented, not profession gear, unless an exact override explicitly says otherwise.

### Robot limbs

Arms:
Economy Arm; Skeleton Arm; KLR Series Arm; Industrial Lifter Arm; Steady Arm; Thief's Arm.

Legs:
Economy Leg; Skeleton Leg; KLR Series Leg; Stealth Leg; Scout Leg.

These are excellent examples of semantic classification from native effects:
- Industrial Lifter Arm => strength/hauling identity.
- Steady Arm => ranged/crossbow identity.
- Thief's Arm => lockpicking/thievery identity.
- Scout Leg => athletics/travel identity.
- Stealth Leg => stealth identity.

For mod-added limbs, existing native stat modifiers should be the primary classifier.

### Lanterns

Lanterns use the belt slot and primarily provide light. They should not receive unrelated profession stats by default.

## What this means for profession gear

Vanilla Kenshi does **not** provide a clean full set for every profession.

Examples that were conceptual, not vanilla item families:
- lab coat,
- chef apron,
- dedicated research tool,
- hoe/farming tool family,
- medical coat,
- smithing hammer as wearable profession gear,
- dedicated crop/ore/medical/tech packs.

Those remain valid additions for this mod's optional FCS content layer and can also be detected on third-party mod items, but they must not be described as existing vanilla equipment.

## Revised classifier priority

Automatic support must not be ID-only. Third-party mods normally add ordinary Kenshi items with no ProfessionGear metadata, so **the item name and description are primary semantic evidence**, constrained by the item type/slot and then resolved by wearer/source context when the item is ambiguous.

Use a scored evidence model:

### 1. Explicit override — strongest
Exact base String ID from `ProfessionGear.rules`.

This handles:
- user corrections,
- known vanilla mappings,
- known popular-mod mappings,
- false positives,
- total conversions.

### 2. Native object type and slot — hard gate
Examples:
- backpack/container,
- headgear,
- shirt/body/leg clothing,
- footwear,
- melee weapon,
- crossbow,
- robot limb,
- belt/lantern.

This prevents nonsense. A weapon called `Farmer's Sword` is still a combat weapon unless its name/description clearly identifies it as an actual work tool. A heavy helmet does not become cooking gear because a cook happens to wear it.

### 3. Item name — primary semantic evidence
This is how unknown mod-added gear is expected to work. Mod authors do not need to know about ProfessionGear.

Useful examples:
- `Green Hat` => plain/light generic workwear candidate; profession comes from wearer/source context.
- `Worker Rags` => generic workwear; Farming/Labouring/Engineering/Cooking are plausible depending on context.
- `Running Shoes` => Athletics/travel candidate from the name alone.
- `Research Goggles` => research/precision candidate.
- plain `Goggles` => ambiguous precision gear; can support Perception/Science/Engineering/Robotics/Turrets depending on context.
- `Pickaxe` => Labouring tool even if a mod implements it using a weapon slot.
- `Chef Knife` is **not** automatically Cooking if it is just a normal combat knife; description/type/context must corroborate that it is a kitchen/work tool.

### 4. Item description — co-primary semantic evidence
Descriptions are especially valuable when names are generic or fictional. A mod can call an item `Mudgrubber Mk II`; if its normal Kenshi description says it is an agricultural harvesting implement, Farming becomes a strong classification without any ProfessionGear-specific metadata.

The local `rebirth.mod` already demonstrates this with the Straw Hat description referring to peasant farmers and desert scouts.

### 5. Existing native mechanics — optional corroboration, not required
Many modded items will have completely ordinary Kenshi stats. That is expected. Existing bonuses can help when present, but **lack of a native Athletics/Perception/etc. bonus must never prevent classification**.

Examples when native mechanics happen to help:
- Athletics bonus can support a travel interpretation,
- Stealth can support stealth gear,
- Perception can support precision/scout gear,
- backpack stacking/weight behavior can support logistics.

These are supporting clues only.

### 6. Semantic tokens used by name/description
Useful for third-party items:
- hoe, sickle, farm, cultivator
- pickaxe, mining, miner
- research, science, laboratory, analyzer
- engineer, construction, tool belt
- medic, doctor, surgical, first aid
- chef, cook, kitchen
- smith, forge, hammer
- scout, ranger
- stealth, ninja, infiltrator
- ore pack, crop pack, medical pack, tech pack, etc.

A name hit must be compatible with the object's slot/type. A sword named "Farmer's Katana" is still a combat weapon unless another strong signal or explicit rule says otherwise.

### 7. NPC/source context — resolves ambiguous legal pools per instance
NPC role, skills, faction and job determine which plausible stat an ambiguous item actually receives.

Example: the same `Worker Rags` base item can produce a Farming roll on a farmer and a Labouring roll on a miner/labourer. The same plain goggles can become Science on a researcher or Perception/Turrets on a turret specialist.

Context cannot override the hard item-type gate. A researcher carrying Samurai Armour does not make Samurai Armour a research coat.

## Mod support

This hybrid system is specifically how new mod gear remains supported.

A mod item does **not** need a known ID and does **not** need any ProfessionGear-style native stat. It only needs to be an ordinary Kenshi item whose name/description/type give us enough context.

Examples:

- `Green Hat`
  - head slot,
  - ordinary cloth item with no special stats,
  - enters the generic workwear pool; wearer/source context can make an individual copy Farming, Labouring, Cooking or Engineering.

- `Running Shoes`
  - footwear,
  - no native Athletics bonus required,
  - the name is enough to make Athletics/travel a strong legal interpretation.

- `Mudgrubber Mk II`
  - ordinary item or weapon-slot tool,
  - description says agricultural implement / harvesting,
  - classifier can infer Farming despite the fictional display name and no pre-existing Farming stat.

- `Plain Goggles`
  - head/face gear,
  - no native Perception stat required,
  - enters a multi-purpose precision pool and context picks the actual affix.

- `Nomad Cargo Frame`
  - backpack/container,
  - name/description say cargo hauling,
  - classifier can infer logistics even if its native backpack numbers are completely vanilla-like.

When evidence is weak or contradictory, the correct answer is **no profession affix**, not a guess.

## Proposed confidence model

Each semantic candidate gets a score.

Example evidence weights:
- exact rule: authoritative
- known curated vanilla/mod ID: +100
- compatible object type/slot: required gate
- strong name semantic match: +35
- strong description semantic match: +35
- corroborating name+description agreement: +20
- native mechanical signature when present: +10
- NPC/source role match: resolves/weights the legal pool rather than inventing a new item identity
- contradictory type: hard reject
- explicitly legendary/unique protected item: hard reject

Suggested behavior:
- >= 50: auto-classify
- 30–49: classify only with corroborating second signal
- < 30: no automatic profession tag
- explicit exclusion: always reject

Exact thresholds should be tested against the real vanilla catalogue plus a sample of installed third-party gear.

## Initial vanilla profession policy

### Safe automatic utility mappings
- Wooden Sandals => Athletics/travel utility.
- Scout Legs => Athletics/travel.
- Stealth Legs => Stealth.
- Thief's Arm => Lockpicking/Thievery.
- Steady Arm => ranged/precision.
- Industrial Lifter Arm => hauling/strength utility.
- Swamp Ninja/Ninja gear => stealth family where native stats support it.
- Perception-positive goggles/hats => scout/precision family.

### Curated but design-driven profession associations
- Straw Hat => Farming is reasonable because the vanilla description explicitly associates it with peasant farmers, but Farming is still a mod-added interpretation rather than a native Farming bonus.
- Square Goggles / Ashlander Stormgoggles => possible Science/Engineering/Robotics candidates only when corroborated by source/NPC context or explicit curated mapping. Vanilla itself only gives them Perception; it does not call them research gear.

### Do not auto-assign profession stats
- generic shirts,
- generic trousers,
- generic armour,
- generic melee weapons,
- generic crossbows,
- generic backpacks.

Profession-specific variants for Cooking, Science, Medicine, Smithing, Engineering, Farming, etc. should come from:
- new FCS content,
- clearly semantic mod-added gear,
- explicit rules,
- or multiple corroborating runtime signals.

## Affix-count progression

Requested progression:
- tiers 0–2: max 1 affix,
- tiers 3–4: up to 2 affixes,
- tiers 5–6: up to 3 affixes.

The item still cannot exceed the number of coherent stats in its legal pool. A Farming-only hoe can remain one-stat even at top tier. A high-end research/engineering item with a three-stat coherent pool can roll up to three.

This is a ceiling, not a guarantee that every high-tier item has three stats.
