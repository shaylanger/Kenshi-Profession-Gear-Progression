# Profession Gear Progression — Full Test Plan

**Status:** installed for testing; the coordinator session runs the game. Per-row state: `INGAME_STATUS.md`; in-game scenarios: `tests/ingame/` (`RUN_ORDER.md`).

**Current offline automation:** `run_tests.bat` passes **5,238 core checks**; `verify_offline.bat` passes **21 required SDK/export symbol checks**; source-contract verification passes **9 checks**; `build_portable.bat`, `package.bat`, and the full readiness gate are green. The numbered plan now extends through **331**. These do not substitute for the in-game rows below.

## Test policy

- Automated tests are run by the agent whenever they can be proven from process output, logs, saved state or deterministic game state.
- Tests marked **SHAY** require visual/UI/game-feel confirmation only.
- Every in-game run archives `ProfessionGear.log`, `profession_gear_affixes.tsv`, `stobe.log` and `KenshiFP.log` before relaunch.
- Use a fixture save/copy. Never overwrite Shay's main save.
- Test vanilla Kenshi first, then coexistence with STOBE/KenshiFP, then third-party compatibility.
- A build pass is not an in-game pass.

## Section A — build/static/core tests

| ID | Automation | Test | Expected |
|---|---|---|---|
| 1 | AUTO | VS2010 portable compile of core | No compile errors |
| 2 | AUTO | VS2010 portable compile of plugin | No compile errors |
| 3 | AUTO | Link DLL with KenshiLib/MyGUI/Ogre | BUILD OK |
| 4 | AUTO | Export table | Undecorated `startPlugin` exists |
| 5 | AUTO | Core regression executable | Exit 0 |
| 6 | AUTO | Quality boundary table | Every boundary maps to intended tier |
| 7 | AUTO | Tier magnitude ranges | Higher tiers never use lower max range |
| 8 | AUTO | Tier affix chances | Non-decreasing from tier 0 through 6 |
| 9 | AUTO | Deterministic RNG | Same key+seed gives identical roll |
| 10 | AUTO | Instance variance | Distinct keys can produce distinct rolls |
| 11 | AUTO | Stackable exclusion | Stackable items receive no affix |
| 12 | AUTO | Serialization round-trip | Record survives serialize/parse identically |
| 13 | AUTO | Malformed sidecar rows | Rejected without crash |
| 14 | AUTO | Unknown stat ID | Rejected |
| 15 | AUTO | Exact exclusion precedence | Excluded ID never classifies |
| 16 | AUTO | Exact override precedence | Explicit tags replace automatic guess |
| 16a | AUTO | Low-tier affix cap | Tiers 0–2 never exceed 1 affix |
| 16b | AUTO | Medium-tier affix cap | Tiers 3–4 never exceed 2 affixes |
| 16c | AUTO | Top-tier affix cap | Tiers 5–6 never exceed 3 affixes |
| 17 | AUTO | Farming classifier | Hoe/farm tools map only to farming pool |
| 18 | AUTO | Mining classifier | Pickaxe/mining tools map to labouring |
| 19 | AUTO | Research classifier | Lab/research items map to science/robotics |
| 20 | AUTO | Engineering classifier | Engineering/construction tools map correctly |
| 21 | AUTO | Robotics classifier | Robotics tools map correctly |
| 22 | AUTO | Medical classifier | Medical tools/clothes/packs map to Medic |
| 23 | AUTO | Weapon smith classifier | Smith item maps to Weapon Smithing |
| 24 | AUTO | Armour smith classifier | Smith item maps to Armour Smithing |
| 25 | AUTO | Crossbow smith classifier | Crossbow smith item maps correctly |
| 26 | AUTO | Cooking classifier | Cooking gear maps to Cooking |
| 27 | AUTO | Travel/scout classifier | Boots/scout gear maps Athletics/Perception |
| 28 | AUTO | Turret classifier | Turret gear maps Turrets/Perception |
| 29 | AUTO | Stealth classifier | Stealth gear maps Stealth/Lockpicking only |
| 30 | AUTO | Ordinary sword negative case | No Farming/Science/etc. affix |
| 30a | AUTO | Plain mod-added hat (e.g. Green Hat) | Generic workwear pool; context chooses profession |
| 30b | AUTO | Same Worker Rags on farmer vs labourer | Same base item rolls Farming vs Labouring by context |
| 30c | AUTO | Plain mod-added goggles | Multi-context Perception/Science/Engineering/Robotics/Turrets pool |
| 30d | AUTO | Running Shoes with no native Athletics stat | Name/type still classify as Athletics/travel |
| 30e | AUTO | Farmer-named combat sword | Farming is allowed because the name strongly signals profession roleplay |
| 30f | AUTO | Weapon-slot Pickaxe / Pitchfork | Labouring/Farming allowed from strong profession semantics while bonus remains equip-only |
| 30g | AUTO | Meitou/Cross/level-100 weapon | Never receives ProfessionGear affix |
| 30h | AUTO | Edge-grade weapon on unique named NPC | Protected from ProfessionGear affixes |
| 30i | AUTO | Installed UWE/GenMod classifier corpus | False positives/negatives reviewed and converted to regression fixtures |

## Section B — plugin startup/hook safety

| ID | Automation | Test | Expected |
|---|---|---|---|
| 31 | AUTO | Load plugin on fixture | Kenshi reaches loaded world |
| 32 | AUTO | Startup log | Version line written once |
| 33 | AUTO | PlayerInterface hook | Hook reports success |
| 34 | AUTO | CharStats getStat hook | Hook reports success |
| 35 | AUTO | Craft completion hook | Hook reports success |
| 36 | AUTO | Inventory weight hook | Hook reports success |
| 37 | AUTO | Tooltip base hook | Hook reports success or documented class-specific fallback |
| 38 | AUTO | Armour tooltip hook | Hook reports success |
| 39 | AUTO | Container tooltip hook | Hook reports success |
| 40 | AUTO | Crossbow tooltip hook | Hook reports success |
| 41 | AUTO | Sword tooltip hook | Hook reports success |
| 42 | AUTO | Idle for 10 minutes game time | No crash, no runaway log growth |
| 43 | AUTO | Pause/unpause/speed 1x/50x | No duplicate generation or instability |
| 44 | AUTO | Save while loaded | No crash/corruption |
| 45 | AUTO | Exit normally | Sidecar fully flushed |

## Section C — per-instance affix generation

| ID | Automation | Test | Expected |
|---|---|---|---|
| 46 | AUTO | First eligible equipped item observed | One record created |
| 47 | AUTO | Same item scanned repeatedly | Record never rerolls |
| 48 | AUTO | Unequipped eligible item in inventory | Record may persist, but **carrying it grants zero bonus** |
| 49 | AUTO | Re-equip same item | Same roll becomes active |
| 50 | AUTO | Drop and pick up same item | Same instance retains same roll |
| 51 | AUTO | Transfer item to squadmate | Same item roll follows item |
| 52 | AUTO | Transfer item to storage then back | Same roll persists |
| 53 | AUTO | Two identical base items | Independent instance records |
| 54 | AUTO | Low quality sample set | Rolls stay in low-tier range |
| 55 | AUTO | High quality sample set | Rolls stay in high-tier range |
| 56 | AUTO | High-tier dual-affix sample | At most MaxAffixes; both contextual |
| 57 | AUTO | Ineligible generic item | No record or empty/no-affix record only |
| 58 | AUTO | Stackable resource | No affix |
| 59 | AUTO | Existing combat sword | No profession garbage affix |
| 60 | AUTO | Explicit rules exclusion | No affix despite matching name |
| 61 | AUTO | Explicit rules override | Correct pool despite misleading name |

## Section D — effective stat behavior

| ID | Automation | Test | Expected |
|---|---|---|---|
| 62 | AUTO | Farming gear equip | Effective Farming increases by rolled percent |
| 63 | AUTO | Farming gear unequip | Effective Farming returns exactly to baseline |
| 64 | AUTO | Labouring gear equip | Effective Labouring increases |
| 65 | AUTO | Science gear equip | Effective Science increases |
| 66 | AUTO | Engineering gear equip | Effective Engineering increases |
| 67 | AUTO | Robotics gear equip | Effective Robotics increases |
| 68 | AUTO | Medic gear equip | Effective Medic increases |
| 69 | AUTO | Weapon Smith gear equip | Effective Weapon Smithing increases |
| 70 | AUTO | Armour Smith gear equip | Effective Armour Smithing increases |
| 71 | AUTO | Crossbow Smith gear equip | Effective Crossbow Smithing increases |
| 72 | AUTO | Cooking gear equip | Effective Cooking increases |
| 73 | AUTO | Turret gear equip | Effective Turrets increases |
| 74 | AUTO | Athletics gear equip | Effective Athletics increases |
| 75 | AUTO | Perception gear equip | Effective Perception increases |
| 76 | AUTO | Stealth gear equip | Effective Stealth increases |
| 77 | AUTO | Lockpick gear equip | Effective Lockpicking increases |
| 78 | AUTO | Multiple same-stat pieces | Percent bonuses aggregate once |
| 79 | AUTO | Mixed profession pieces | Each stat receives only its own affixes |
| 80 | AUTO | `getStat(..., true)` path | Base/unmodified stat remains unchanged |
| 81 | AUTO | Earn XP while geared | Stored base skill rises normally; gear bonus is not baked in |
| 82 | AUTO | Remove all gear after XP | Base skill equals trained value, not boosted value |
| 83 | AUTO | Very high skill + gear | Effective stat clamps safely at configured hard cap |
| 84 | AUTO | Combat unrelated stats | Melee attack/defence/toughness unchanged by profession affixes |

## Section E — player crafting

| ID | Automation | Test | Expected |
|---|---|---|---|
| 85 | AUTO | Craft one eligible item | Roll created at finished-item hook |
| 86 | AUTO | Craft two identical same-tier items | Rolls may differ, both valid for tier |
| 87 | AUTO | Craft different-quality items | Higher-quality item uses stronger range |
| 88 | AUTO | Craft ineligible item | No profession affix |
| 89 | AUTO | Critical-success craft | Actual finished quality drives tier |
| 90 | AUTO | Crafter strong in matching profession | Matching contextual pool/chance applied |
| 91 | AUTO | Crafted item immediately equipped | Bonus active without relaunch |
| 92 | AUTO | Crafted item save/reload | Exact same roll retained |

## Section F — NPC context and loot

| ID | Automation | Test | Expected |
|---|---|---|---|
| 93 | AUTO | Farmer with hoe/work gear | Farming-affix chance elevated |
| 94 | AUTO | Miner/labourer with pickaxe | Labouring-affix chance elevated |
| 95 | AUTO | Researcher with lab gear | Science/Robotics pool only |
| 96 | AUTO | Engineer | Engineering/Labouring pool only |
| 97 | AUTO | Medic | Medic pool only |
| 98 | AUTO | Smith | Relevant smith pool only |
| 99 | AUTO | Cook | Cooking pool only |
| 100 | AUTO | Turret operator | Turrets/Perception pool only |
| 101 | AUTO | Generic combat soldier | Profession affixes rare/absent unless gear itself qualifies |
| 102 | AUTO | Slave sample | Special-affix frequency strongly suppressed |
| 103 | AUTO | Very low-skill poor NPC sample | Frequency suppressed |
| 104 | AUTO | High-skill specialist sample | Matching affix frequency elevated |
| 105 | AUTO | Kill/loot affixed NPC | Loot keeps same roll |
| 106 | AUTO | NPC streams out/in | Item does not reroll |
| 107 | AUTO | NPC save/reload | Item does not reroll |
| 108 | AUTO | Named/unique NPC | Stable behavior and no duplication |

## Section G — specialist backpacks

| ID | Automation | Test | Expected |
|---|---|---|---|
| 109 | AUTO | Ore pack + ore only | Matching contents receive strong effective weight reduction |
| 110 | AUTO | Ore pack + food | Food retains normal weight |
| 111 | AUTO | Ore pack mixed load | Only ore share gets specialist ratio |
| 112 | AUTO | Crop pack + Wheatstraw/Cactus/Greenfruit | Crop share reduced |
| 113 | AUTO | Construction pack + Building Materials/Iron Plates | Construction share reduced |
| 114 | AUTO | Medical pack + first-aid/splints/repair kits | Medical share reduced |
| 115 | AUTO | Tech pack + books/research/AI cores | Tech share reduced |
| 116 | AUTO | Trade pack + trade goods | Trade-good share reduced |
| 117 | AUTO | Generic backpack same contents | Vanilla total unchanged |
| 118 | AUTO | Empty specialist pack | Zero/vanilla weight path; no divide-by-zero |
| 119 | AUTO | Move item into/out of specialist pack | Weight recalculates immediately |
| 120 | AUTO | Nested/invalid ownership edge | Hook falls back to vanilla result |
| 121 | AUTO | Existing vanilla stack bonuses | Specialist calculation does not break stacking |
| 122 | AUTO | Existing backpack weight multiplier | Specialist ratio composes with vanilla multiplier |

## Section H — persistence/save/import

| ID | Automation | Test | Expected |
|---|---|---|---|
| 123 | AUTO | Save/reload same save | Exact rolls persist |
| 124 | AUTO | Restart Kenshi | Sidecar reloads |
| 125 | AUTO | Sidecar missing | Fresh records generate safely |
| 126 | AUTO | Empty sidecar | Safe startup |
| 127 | AUTO | Corrupt row among valid rows | Bad row ignored; valid rows load |
| 128 | AUTO | Duplicate key rows | Deterministic last/defined behavior; no crash |
| 129 | AUTO | Item destroyed | Stale sidecar row causes no runtime error |
| 130 | AUTO | Large sidecar 10k records | Startup remains acceptable |
| 131 | AUTO | Save copied under new name | Existing item handles checked for stability |
| 132 | AUTO | Kenshi import | Define/verify whether handles persist; document reroll policy if not |
| 133 | AUTO | New game | Old save's unreachable records do not affect new items |
| 134 | AUTO | Autosave/manual save cycling | No sidecar corruption |

## Section I — tooltip/UI

| ID | Automation | Test | Expected |
|---|---|---|---|
| 135 | SHAY | Hover affixed armour | Profession section visible and readable |
| 136 | SHAY | Hover affixed backpack | Profession section visible |
| 137 | SHAY | Hover affixed weapon/tool | Profession section visible |
| 138 | SHAY | Hover non-affixed item | No empty/noisy profession section |
| 139 | SHAY | Dual-affix item | Both bonuses shown once |
| 140 | SHAY | Large/decimal roll | Formatting is clean |
| 141 | SHAY | Inventory/shop/loot tooltip contexts | No duplicate rows or layout corruption |

## Section J — compatibility/coexistence

| ID | Automation | Test | Expected |
|---|---|---|---|
| 142 | AUTO | STOBE + ProfessionGear loaded | Both initialize and remain functional |
| 143 | AUTO | KenshiFP + ProfessionGear loaded | Both initialize and remain functional |
| 144 | AUTO | STOBE + KenshiFP + ProfessionGear | No hook-chain crash |
| 145 | AUTO | Existing mod-added hoe | Auto-classifies and rolls |
| 146 | AUTO | Existing mod-added goggles | Contextual classification only |
| 147 | AUTO | Existing mod-added backpack | Exact rule can specialize it |
| 148 | AUTO | Misleading third-party item name | Rules exclusion fixes false positive |
| 149 | AUTO | AutoClassify=false | Only explicit-tag items participate |
| 150 | AUTO | Plugin disabled via Enabled=false | Hooks remain safe and no new affixes/stat effects occur |

## Section K — performance/stability

| ID | Automation | Test | Expected |
|---|---|---|---|
| 151 | AUTO | 100 loaded characters | 1-second scan does not create visible hitch |
| 152 | AUTO | 300+ loaded characters | No runaway CPU/logging |
| 153 | AUTO | Inventory with many items | Stat lookup remains responsive |
| 154 | AUTO | Repeated stat queries | No recursive hook/deadlock |
| 155 | AUTO | Repeated tooltip opening | No memory growth/crash |
| 156 | AUTO | 50x speed work session | Stable for 30 real minutes |
| 157 | AUTO | Combat while profession gear equipped | No unrelated combat regression |
| 158 | AUTO | Rapid equip/unequip | No stale bonus |
| 159 | AUTO | Rapid inventory transfer | No stale weight/bonus |
| 160 | AUTO | Save during active crafting | Finished item rolls once only |

## Section L — balance calibration

Detailed methodology and measurement matrix: `BALANCE_TEST_PLAN.md`.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 161–176 | AUTO | Farming baseline, response curve, Shoddy min/mid/max and full-set tiers | Real throughput curve measured; final Farming ranges fitted from data |
| 177–183 | AUTO | Labouring/mining baseline, response curve and full-set progression | Real throughput curve measured; final Labouring ranges fitted from data |
| 184–192 | AUTO | Science, Engineering, Robotics, Cooking, smithing, Medic and Turret calibration | Profession-specific response curves measured |
| 193–199 | AUTO | Athletics, Swimming, Stealth, Lockpicking, Assassination, Thievery, Perception | Utility/probability effects measured with deterministic/repeated fixtures |
| 200–210 | AUTO | Cross-profession fitting, skill-vs-gear sanity, full-set caps and final freeze | v1 ranges based on measured gameplay rather than placeholder percentages |

Current tier values are **provisional until Section L is run in-game**.

## Release gate

The mod is not release-ready until:
1. all AUTO tests that can run in the fixture environment pass,
2. all hook symbols are confirmed on the installed Kenshi build,
3. stable item identity survives save/reload and normal transfers,
4. job calculations demonstrably use the hooked effective profession stats,
5. specialist backpack totals are verified against vanilla UI weight,
6. coexistence with the current STOBE + KenshiFP setup passes,
7. SHAY tooltip/UI rows 135–141 are accepted.

Any failure gets a bug number and a regression row before the fix is considered complete.

## Section M — live distribution, shop stock, and recent classifier assumptions

These tests exist specifically so design assumptions are not left as informal "needs testing" notes.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 211 | AUTO | Generic workwear in trader inventory | Item may roll one of its legal profession stats even with no wearer profession context |
| 212 | AUTO | Generic goggles in trader inventory | Item may roll only from Perception/Science/Engineering/Robotics/Turrets pool |
| 213 | AUTO | Loose generic workwear discovered as world/container loot | May roll a random profession from its legal pool at the reduced world-loot chance |
| 214 | AUTO | Buy profession-affixed generic item from trader | Exact affix persists after purchase/transfer |
| 215 | AUTO | Purchased item carried but not equipped | Zero effective-stat bonus |
| 216 | AUTO | Purchased item equipped | Stored affix becomes active exactly once |
| 217 | AUTO | Trader stock refresh/restock | New item instances may roll independently; existing surviving instances do not reroll |
| 218 | AUTO | Trader character's personally equipped gear | Uses wearer role context, not random shop-stock context |
| 219 | AUTO | Detect whether Kenshi shop stock actually resides in trader Character inventory | Runtime trader-source detection covers real vendor stock, or alternate shop-container hook is documented/implemented |
| 220 | AUTO | Vendor scan: Farming gear availability | Qualifying Farming items appear at measurable frequency |
| 221 | AUTO | Vendor scan: Labouring gear availability | Qualifying Labouring items appear at measurable frequency |
| 222 | AUTO | Vendor scan: Medic gear availability | Qualifying Medic items appear at measurable frequency |
| 223 | AUTO | Vendor scan: Engineering gear availability | Qualifying Engineering items appear at measurable frequency |
| 224 | AUTO | Vendor scan: Science/Research gear availability | Qualifying Science items appear at measurable frequency |
| 225 | AUTO | Vendor scan: Robotics gear availability | Qualifying Robotics items appear at measurable frequency |
| 226 | AUTO | Vendor scan: Cooking gear availability | Qualifying Cooking items appear at measurable frequency |
| 227 | AUTO | Vendor scan: Smithing gear availability | Weapon/Armour/Crossbow Smithing items appear at measurable frequency |
| 228 | AUTO | NPC/loot distribution by profession | Role-context gear appears on appropriate NPCs without unrelated profession rolls |
| 229 | AUTO | UWE Pickaxe/Sickle/Pitchfork-like weapon-class tools | Affix activates only while actually equipped |
| 230 | AUTO | Non-equippable decorative/workstation tools (e.g. GenMod Pickaxe_Deco) | Never grant character stat bonuses |
| 231 | AUTO | Farmer-named / Engineer-named / Medic-named / Chef-named weapons | Strong profession semantics produce only coherent profession affixes |
| 232 | AUTO | Ordinary combat weapons across vanilla/UWE corpus | No profession affixes without profession semantics |
| 233 | AUTO | Meitou/Cross/level-100 weapon corpus | Never modified |
| 234 | AUTO | Edge-grade weapon on unique named NPC corpus | Protected as designed |
| 235 | AUTO | Generic workwear profession fallback distribution | Farmer/Medic/Engineer/Cook/Researcher/etc. resolve to their matching profession, not random alternatives |
| 236 | AUTO | Generic goggles profession fallback distribution | Research/Turret/Engineering/Robotics/Perception contexts resolve correctly |
| 237 | AUTO | Shop-stock profession distribution | No illegal stats; distribution is broad enough to make browsing shops worthwhile |
| 238 | AUTO | Vendor inventory performance with large mod load | No noticeable stall from classification/record creation during shop open/restock |
| 239 | AUTO | Save/load after buying trader-generated profession gear | Item identity and affix survive save/load |
| 240 | AUTO | Import/new-game behavior for trader-generated gear | No stale sidecar attachment to unrelated replacement items |

Release note: if tests 220–227 show a profession is technically possible but too rare to encounter naturally, fix distribution with curated mappings or optional FCS profession items. Do not solve rarity by assigning unrelated gear nonsensical stats.

## Section N — exploration/world-loot generation

| ID | Automation | Test | Expected |
|---|---|---|---|
| 241 | AUTO | Previously unseen eligible item picked up from ruin chest | Classified as world loot, not from player's profession |
| 242 | AUTO | World-loot generic Rag Shirt | Can roll only from generic workwear pool |
| 243 | AUTO | World-loot generic goggles | Can roll only from goggles pool |
| 244 | AUTO | WorldLootMultiplier=0 | Contextless exploration gear never rolls an affix |
| 245 | AUTO | WorldLootMultiplier=0.50 statistical sample | Observed eligible-roll frequency is approximately half normal tier chance within tolerance |
| 246 | AUTO | Same ruin item dropped and picked up repeatedly | Never rerolls after first record is created |
| 247 | AUTO | Two characters with different professions pick identical unseen world-loot fixtures | Roll distribution is independent of player profession |
| 248 | AUTO | Inspect unopened chest before pickup, if game exposes item objects to plugin | If items are discoverable pre-pickup, affix is stable before/after pickup; otherwise pickup-time world-loot generation is documented and verified |
| 249 | AUTO | Starting/player-owned pre-existing eligible item with no record | Does not inherit player's profession merely because it is first scanned on a player character |
| 250 | AUTO | World-loot profession distribution across ruin/chest corpus | Exploration yields a useful mix of coherent profession gear without excessive frequency |

## Section O — approved roadmap validation

These tests correspond to approved future goals so roadmap work is not left as untracked prose.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 251 | AUTO | Explicitly unique non-weapon item | No ProfessionGear affix |
| 252 | AUTO | Explicitly unique weapon below Meitou quality | No ProfessionGear affix |
| 253 | AUTO | Normal non-unique high-quality item | May still participate if semantically eligible |
| 254 | AUTO | Running/travel footwear utility effect | Uses appropriate native/effective movement mechanic without altering base skill permanently |
| 255 | AUTO | Load-bearing/hauling gear utility effect | Encumbrance/carry benefit applies only while equipped |
| 256 | AUTO | Swimming-oriented gear | Swimming utility applies only when semantically valid |
| 257 | AUTO | Native utility modifier coexistence | ProfessionGear does not overwrite vanilla armour/container modifiers |
| 258 | AUTO | Generated Farmer's Sword base record | Uses reused asset, classifies Farming, supports per-instance rolls |
| 259 | AUTO | Generated Miner's Backpack base record | Uses reused asset, classifies mining/logistics, supports specialist behavior |
| 260 | AUTO | Generated Traveler's Sandals base record | Uses reused asset, classifies travel/Athletics utility |
| 261 | AUTO | Generated Engineer gear base records | Appear in appropriate content pool and classify Engineering |
| 262 | AUTO | Generated Field Medic gear base records | Appear in appropriate content pool and classify Medic |
| 263 | AUTO | Generated Research gear base records | Appear in appropriate content pool and classify Science/Robotics as intended |
| 264 | AUTO | Generated Cooking gear base records | Appear in appropriate content pool and classify Cooking |
| 265 | AUTO | Generated Smithing gear base records | Classify only the intended smithing families |
| 266 | AUTO | Generated profession item vendor distribution | Correct shops can stock variants without requiring runtime-only injection hacks |
| 267 | AUTO | Generated profession item ruin/loot distribution | Exploration can yield generated profession variants |
| 268 | AUTO | Generated profession item NPC distribution | Profession/faction-appropriate NPCs can spawn with variants |
| 269 | AUTO | Generated profession item crafting/blueprints | Player can obtain/craft variants through normal Kenshi systems where intended |
| 270 | AUTO | Generated item runtime affix integration | New FCS/content variants use the same per-instance affix system as vanilla/modded gear |
| 271 | AUTO | Specialist pack selective stacking feasibility | Implement only if intended cargo can stack more without boosting unrelated contents |
| 272 | AUTO | Specialist pack fallback | If selective stacking is unsafe, weight specialization remains correct and stacking stays vanilla |


## Section P — pre-install hardening / packaging

| ID | Automation | Test | Expected |
|---|---|---|---|
| 273 | AUTO | Quantity-one stack-capable item | Runtime uses Kenshi `isStackable(section)`; no profession affix even when current quantity is 1 |
| 274 | AUTO | Existing sidecar key reused by different base item | Base-ID mismatch invalidates stale record and rerolls safely |
| 275 | AUTO | Character bonus-cache identity | Full Kenshi handle string is used, not serial alone |
| 276 | AUTO | Inventory callback reentrancy | Nested add/remove/update notifications do not recursively rescan indefinitely |
| 277 | AUTO | Tooltip base + derived hook chain | `Profession Gear` section appears at most once |
| 278 | AUTO | Invalid negative config multipliers / MaxAffixes out of range | Values normalize to safe limits |
| 279 | AUTO | Sidecar replacement | Database update uses atomic replace; failed replace leaves dirty flag for retry |
| 280 | AUTO | Plugin entry called twice | Hooks initialize only once |
| 281 | AUTO | Generic vanilla/modded backpack | May roll Athletics utility but gets no specialist cargo weight reduction |
| 282 | AUTO | Explicit specialist backpack | Uses specialist tag and does not fall back to generic pack classification |
| 283 | AUTO | Strong themed clothing vs generic fallback | Assassin/Medic/Farming/etc. semantics prevent unrelated generic-workwear rolls |
| 284 | AUTO | Utility classifier corpus | Assassin, Thief, Ninja/Stealth, Swim, Travel and Hauling tags map only to coherent utility stats |
| 285 | AUTO | Mechanical-named ordinary weapon | Does not false-match the Engineer `mechanic` token |
| 286 | AUTO | Package includes minimal Kenshi .mod stub | Launcher-recognizable `ProfessionGearProgression.mod` exists and is non-empty |
| 287 | AUTO | Installer invoked without explicit `-Install` | Refuses to touch Kenshi |
| 288 | AUTO | Package verifier | Required DLL/.mod/manifest/config/rules all present; runtime sidecar/log absent |
| 289 | AUTO | Startup diagnostics | Log records version, normalized config and rule counts before hook results |
| 290 | AUTO | Existing test install present | Installer backs it up before copying a replacement |


## Section Q — equip-only specialist utilities / semantic boundary regressions

| ID | Automation | Test | Expected |
|---|---|---|---|
| 291 | AUTO | Specialist backpack not equipped | Extra category/hauling weight reduction is inactive |
| 292 | AUTO | Specialist backpack equipped | Extra category/hauling weight reduction activates and composes with vanilla total |
| 293 | AUTO | Assassin's Blade / Thief's Dagger / Scout Sword | Weapon class does not block strongly signaled utility-role affixes |
| 294 | AUTO | Running Shoes semantic boundary | `shoe`/ `shoes` never false-match the Farming `hoe` token |
| 295 | AUTO | Dragon Armour semantic boundary | `dragon` never false-matches generic `rag` workwear |
| 296 | AUTO | Mineral-named gear semantic boundary | `mineral` never false-matches `miner` |
| 297 | AUTO | Visor semantic boundary | unrelated words containing `visor` do not classify as goggles/research gear |
| 298 | AUTO | Protected/excluded stale sidecar row | Unique or explicit-exclusion item suppresses/removes old persisted affix |
| 299 | AUTO | Generic backpack vs specialist pack | Generic pack may roll Athletics but receives no specialist contents-weight reduction |
| 300 | AUTO | Verbose generation diagnostics | New roll log contains source, key/base ID/name, tier, tags and affixes |


## Section R — profession-named backpack specialization

| ID | Automation | Test | Expected |
|---|---|---|---|
| 301 | AUTO | Miner's/Miners Backpack | Classified as ore/mining specialist; not generic pack fallback |
| 302 | AUTO | Farmer's/Farmers Backpack | Classified as crop/farming specialist; not generic pack fallback |
| 303 | AUTO | Field Medic/Doctor Backpack | Classified as medical specialist; not generic pack fallback |
| 304 | AUTO | Engineer/Builder Pack | Classified as construction specialist; not generic pack fallback |
| 305 | AUTO | Research/Science/Robotics Satchel | Classified as tech specialist; not generic pack fallback |


## Section S — final design-audit regression

| ID | Automation | Test | Expected |
|---|---|---|---|
| 306 | AUTO | Craft hook ordering / critical-success quality | Kenshi original `addFinishedCraftItem` completes first; ProfessionGear then reads the final item quality/model and rolls exactly once |

## Section T — RE_Kenshi loader contract

| ID | Automation | Test | Expected |
|---|---|---|---|
| 307 | AUTO | Native plugin entry export | DLL exports C++-mangled `?startPlugin@@YAXXZ`; plain `extern "C" startPlugin` is rejected because RE_Kenshi resolves the mangled C++ symbol |


## Section U — live non-equippable false-positive regression

These rows were added from the first forced live scan on 2026-10-02.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 308 | AUTO | Chewing Tobacco / ordinary consumable with work-related description words | Non-Gear/non-ContainerItem is rejected before record generation |
| 309 | AUTO | Bolts [Regulars] / ammunition | Non-equippable ammo never receives ProfessionGear record/tooltip |
| 310 | AUTO | Basic/Standard First Aid Kit / Splint Kit | Medical consumables are not treated as equipable Medic gear |
| 311 | AUTO | Medical Supplies / research-related trade item | Non-equippable supplies cannot roll Science/Robotics/Medic affixes |


## Section V — equipped-section inventory regression

These rows were added from the first Phase 2 live equip test on 2026-10-02.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 312 | AUTO | Equipped belt/armour/weapon/backpack item leaves Inventory::getAllItems() | ProfessionGear still discovers the item by scanning all character inventory sections and applies its affix while equipped |
| 313 | AUTO | Harness item lookup after equip | stobe-auto iteminfo/unequip finds items in equipped sections, not only the main inventory list |


## Section W — save-native persistent item identity

Added after live test 123 proved Kenshi runtime item handles change across ordinary save/reload.

| ID | Automation | Test | Expected |
|---|---|---|---|
| 314 | AUTO | Eligible item serialized into inventory save state | `ProfessionGearPersistentId` is written into the item's serialized `GameData` |
| 315 | AUTO | Same item loaded from inventory save state | Serialized persistent ID is restored and bound to the new runtime handle |
| 316 | AUTO | Runtime handle changes across exact save/reload | Sidecar lookup still resolves the same persistent ID and exact affix record |
| 317 | AUTO | Equipped affixed item save/reload | Same affix remains active after reload even though runtime handle changes |
| 318 | AUTO | Full Kenshi process restart + reload | Persistent ID/affix survives process restart, not just in-process reload |
| 319 | AUTO | Pre-v2 runtime-handle sidecar rows | Legacy rows are ignored; they cannot attach to unrelated new runtime items |

## Section X — live contextual-generic coherence regression

| ID | Automation | Test | Expected |
|---|---|---|---|
| 320 | AUTO | Top-tier generic workwear/goggles on a known-role NPC | Matching NPC role narrows the generic roll pool; no unrelated second/third profession affixes are added |

## Section Y — 2026-10-03 offline review fixes (regression rows)

| ID | Automation | Test | Expected |
|---|---|---|---|
| 321 | AUTO | Sidecar row with no affix, CRLF line end, full restart | Record loads (not dropped), item does not roll again |
| 322 | AUTO | Weapon whose only profession word is flavour description (Staff "poor farmers", Bardiche "slay thieves", Ronin Hatchet "robotic tools") | No profession tag; weapons classify by name only (Scythe by name = farming) |
| 323 | AUTO | Natural shop stock in the trader's faction storage within 60 | Scanned as trader stock (source=trader, random legal pool), records exist before purchase |
| 324 | AUTO | Purchase through `Inventory::buyItem` hands the buyer a copy | Copy keeps the shop item's persistent ID and affixes; no world-loot reroll |
| 325 | AUTO | NPC whose best profession skill is below 12 | Counts as poor (PoorNpcMultiplier); shop stock / world loot / crafted items are not judged by the owner's skills |
| 326 | AUTO | Enabled=false | No record is created from any path (scan, test commands, crafting) |
| 327 | AUTO | Item with an old record that is not eligible now (rule change, AutoClassify off, old classifier) | Record kept in the sidecar, gives no bonus and no tooltip; harness-forced records stay active |
| 328 | AUTO | Vanilla "Traders Backpack Medium/Large", "Old Traders backpack" | Trade pack (PACK_TRADE), not generic |
| 329 | AUTO | Craft whose finished item in the bench output is another instance than the one passed to `addFinishedCraftItem` | The item in the output gets the crafted roll (crafter context, final quality); nothing is left unbound to roll later as world loot |
| 330 | AUTO | Armour/weapon whose text mentions traders/backpack/miner (Square Goggles "standard issue in the Traders Guild" in shop storage) | Never a specialist pack tag; rolls only from its own pool |
| 331 | AUTO | Worker job with equipped Labouring/Farming gear (JobOperateScaling default on) | Work per tick and output rise by the equipped bonus (live m16: raw skill path x1.05, with scaling x1.59); ini JobOperateScaling=false turns it off |
