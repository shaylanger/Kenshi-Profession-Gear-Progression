# Profession Gear Progression — Design-to-Implementation Audit

**Audit date:** 2026-10-02
**Purpose:** Verify the current pretest build against Shay's full design/context before first installation.

## Status meanings

- **IMPLEMENTED** — code/config/package exists now; live Kenshi may still need to prove the ABI/gameplay path.
- **LIVE PROOF REQUIRED** — implementation exists, but Kenshi behavior cannot be certified offline.
- **DEFERRED BY DESIGN** — Shay explicitly approved this as a later goal and it is not a blocker for the first runtime test.
- **BALANCE PENDING** — mechanic exists, but the numbers are intentionally provisional until measured in-game.

## Core design audit

| Requirement from design / conversation | Status | Current implementation | Validation |
|---|---|---|---|
| Standalone mod, no STOBE/KenshiFP dependency | IMPLEMENTED | Separate repo/DLL/package | Tests 143–150, coexistence later |
| Existing + mod-added gear support | IMPLEMENTED | Semantic name/description + native item type/slot + exact rules | 10–30, 142–149, 229–236, 283–305 |
| Item names are important for unknown mod gear | IMPLEMENTED | Word-aware semantic classifier | 231–236, 283–305 |
| Weird mod names should not match from one accidental substring | IMPLEMENTED | Word boundaries + strong-semantic precedence | 285, 294–297 |
| Explicit include/exclude rules | IMPLEMENTED | ProfessionGear.rules | 12–14, 148–150 |
| Farmer's Sword / Pitchfork can get Farming | IMPLEMENTED | Profession-role weapons allowed when name/description is strong | 231, 293 |
| Utility-role weapons can be coherent too | IMPLEMENTED | Assassin/Thief/Scout semantic weapons | 293 |
| Generic combat weapons do not get profession stats | IMPLEMENTED | Weapon early-reject without strong semantic role | 232–234 |
| Unique/special named item instances protected | IMPLEMENTED | Item::isUnique + Meitou/Cross/legendary protection | 251–253, 298 |
| Multiple stats on one item | IMPLEMENTED | tier caps: low 1, medium up to 2, top up to 3 | 17–21, 54–61 |
| Quality/tier affects chance and magnitude | IMPLEMENTED / BALANCE PENDING | 7 affix tiers; weapons use 15-grade mapping | 15–21, 54–61, 161–210 |
| Weapon grades handled separately from armour quality | IMPLEMENTED | WeaponGradeRank + ProgressionTier | 17–21 |
| Random per-instance rolls | IMPLEMENTED | deterministic per-instance seed + persisted AffixRecord | 46–61 |
| Same base item can roll differently | IMPLEMENTED | instance identity drives roll | 46–61 |
| Not every eligible item is special | IMPLEMENTED / BALANCE PENDING | tier probability + source multipliers | 54–61, 161–210 |
| Player-crafted gear rolls after Kenshi determines final quality | IMPLEMENTED | original addFinishedCraftItem runs first, then EnsureRecord | 85–92, 306 |
| Bonuses only work while equipped | IMPLEMENTED / LIVE PROOF REQUIRED | equipped cache only; specialist pack effect requires equipped pack | 62–84, 214–216, 291–292 |
| Base skill/XP must not be permanently changed | IMPLEMENTED / LIVE PROOF REQUIRED | getStat effective layer only; unmodified bypass | 79–84 |
| Farming | IMPLEMENTED / LIVE PROOF REQUIRED | Farming stat + tool/head/crop/generic pools | 62+, 93, 161–176 |
| Labouring/mining | IMPLEMENTED / LIVE PROOF REQUIRED | Labouring + mining/work gear | 94, 177–183 |
| Science/research | IMPLEMENTED / LIVE PROOF REQUIRED | Science/Robotics research pools | 95, 184+ |
| Engineering/construction | IMPLEMENTED / LIVE PROOF REQUIRED | Engineering/Labouring pools | 96, 184+ |
| Robotics | IMPLEMENTED / LIVE PROOF REQUIRED | Robotics/Engineering pools | 184+ |
| Medic | IMPLEMENTED / LIVE PROOF REQUIRED | Medic pools | 97, 184+ |
| Cooking | IMPLEMENTED / LIVE PROOF REQUIRED | Cooking pools | 99, 184+ |
| Weapon/Armour/Crossbow smithing | IMPLEMENTED / LIVE PROOF REQUIRED | all three profession stats | 98, 184+ |
| Turrets / Perception | IMPLEMENTED / LIVE PROOF REQUIRED | Turret gear -> Turrets + Perception | 100, 193–199 |
| Athletics / travel | IMPLEMENTED / LIVE PROOF REQUIRED | travel gear / generic packs -> Athletics (+ Perception for scout) | 193–199, 254, 281 |
| Swimming | IMPLEMENTED / LIVE PROOF REQUIRED | swim/diving gear -> Swimming | 193–199, 256, 284 |
| Stealth / infiltration | IMPLEMENTED / LIVE PROOF REQUIRED | Stealth + Lockpicking | 193–199, 284 |
| Assassination | IMPLEMENTED / LIVE PROOF REQUIRED | Assassin gear -> Stealth + Assassination | 193–199, 284 |
| Thievery / lockpicking | IMPLEMENTED / LIVE PROOF REQUIRED | Thief gear -> Stealth + Lockpicking + Thievery | 193–199, 284 |
| Generic rags/workwear can resolve to different professions by context | IMPLEMENTED / LIVE PROOF REQUIRED | broad legal pool, wearer role preference, trader/world fallback | 211, 213, 235 |
| Generic goggles can be Perception/research/engineering/etc. | IMPLEMENTED / LIVE PROOF REQUIRED | Perception/Science/Engineering/Robotics/Turrets pool | 212, 236 |
| Trader stock can contain useful profession variants | IMPLEMENTED / LIVE PROOF REQUIRED | trader-source random legal pool | 211–227 |
| World/chest loot can contain random profession variants | IMPLEMENTED / LIVE PROOF REQUIRED | player-first-seen eligible gear treated as world loot | 241–250 |
| World loot must not adapt to player's profession | IMPLEMENTED / LIVE PROOF REQUIRED | player owner clears role and sets worldLootSource | 241, 247, 249 |
| NPC role should bias gear | IMPLEMENTED / LIVE PROOF REQUIRED | strongest profession skill + slave/wealth + trader + unique context | 93–108, 228 |
| Poor/slave NPCs less likely to get special gear | IMPLEMENTED / BALANCE PENDING | poor/slave multiplier | 102–104 |
| NPC gear persists when looted/streamed | IMPLEMENTED / LIVE PROOF REQUIRED | sidecar keyed by item handle/base | 105–107 |
| Backpacks have profession/logistics niches | IMPLEMENTED / LIVE PROOF REQUIRED | ore/crop/construction/medical/trade/tech/hauling/generic tags | 109–122, 291–305 |
| Ore/crop/etc. selective weight specialization | IMPLEMENTED / LIVE PROOF REQUIRED | contained-item weighted multiplier | 109–122 |
| Hauling/carry support | IMPLEMENTED / LIVE PROOF REQUIRED | all-cargo hauling-pack weight reduction + Athletics | 255, 284, 291–292 |
| Existing vanilla backpack multipliers must compose | IMPLEMENTED / LIVE PROOF REQUIRED | modifier applied as ratio on original total | 121–122 |
| Specialist stacking | DEFERRED BY DESIGN | current runtime API is section-wide; do not boost unrelated cargo | 271–272 |
| Direct movement-speed / encumbrance native paths where needed | LIVE PROOF REQUIRED | current safe implementation uses Athletics + pack weight; live test decides whether direct native hooks are necessary | 193–199, 254–257 |
| Tooltips show per-instance affixes | IMPLEMENTED / LIVE PROOF REQUIRED | original tooltip + one Profession Gear section | 135–141, 277 |
| Persistence through save/load | IMPLEMENTED / LIVE PROOF REQUIRED | atomic TSV sidecar | 123–134 |
| Handle reuse must not attach wrong roll | IMPLEMENTED | base-ID validation + stale-row invalidation | 274, 298 |
| Stackable resources/consumables excluded | IMPLEMENTED | real isStackable(section) detection | 22, 273 |
| Auto support for profession-named backpacks | IMPLEMENTED | Miner/Farmer/Medic/Engineer/Research pack semantics | 301–305 |
| New profession-themed items such as Farmer's Sword / Miner's Backpack / Traveler's Sandals | DEFERRED BY DESIGN | formal FCS/content-generation roadmap, existing classifier already understands these names | 258–270 |
| Normal vendor/NPC/loot/crafting distribution for newly generated FCS variants | DEFERRED BY DESIGN | roadmap after runtime/distribution gaps are measured | 266–270 |
| Balance from real throughput rather than intuition | BALANCE PENDING | 50-test calibration matrix + analyzer | 161–210 |
| STOBE/KenshiFP coexistence | LIVE PROOF REQUIRED | no dependency; shared hooks must be tested | 143–150 |
| Performance at large NPC/item counts | LIVE PROOF REQUIRED | cached 1-second scan + event callbacks | 151–160 |

## Important audit conclusion

The first-test build contains the complete **runtime framework** Shay asked to test: classification, random per-instance progression, tier/quality progression, NPC/trader/world/crafting sources, equip-only effective profession/utility effects, persistence, tooltips, and specialist weight behavior.

Two feature families are intentionally **not blockers for first runtime testing** because Shay explicitly moved them to later goals:

1. creating additional FCS profession-themed base items and distributing them through shops/NPCs/blueprints/loot,
2. category-selective specialist backpack stacking if it cannot be done safely with the runtime API.

The current percentages and source frequencies are also intentionally not final; the live plan measures them.

## Live-test decision points

The first live run must answer these architecture questions before final release:

1. Do real Farming/Labouring/Science/etc. jobs consume the hooked effective CharStats value?
2. Does Athletics provide the desired travel-speed progression, or does movement need a direct native multiplier hook?
3. Is pack weight reduction sufficient for hauling/encumbrance gameplay, or is a direct encumbrance/carry hook still useful?
4. Does actual trader stock live on the trader Character inventory or another shop container?
5. Are unopened ruin/chest item instances visible before pickup, or should world affixes intentionally initialize on first pickup?
6. Are Kenshi item handles stable enough across save/load/import for sidecar identity?
7. Does tooltip hook chaining coexist cleanly with the installed plugin stack?

Every decision point above has an existing numbered test.
