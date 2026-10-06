# Profession Gear Progression — Balance Calibration Plan

## Goal

Replace placeholder affix ranges with numbers derived from **actual Kenshi job throughput**.

The current tier ranges are implementation defaults only. Final ranges should be chosen after controlled measurements show how an effective-stat bonus changes real work speed for each profession.

## Core balance principles

1. Gear must help, but skill training remains the main source of competence.
2. One item should be noticeable but not transformative.
3. A coherent full profession set should feel meaningfully better than ordinary gear.
4. A low-skill worker in elite gear should not outperform a highly trained worker in ordinary gear.
5. Higher tiers should improve expected output without making every lower-tier roll obsolete.
6. Good rolls at one tier may overlap weak rolls at the next tier.
7. Balance is based on **real output/time**, not merely displayed skill.

## Target bands to validate, not assume

These are hypotheses for testing:

| Loadout | Initial target real throughput gain |
|---|---:|
| One Shoddy piece | 2–5% |
| Full Shoddy set | 10–15% |
| Full Standard/Mid set | 15–25% |
| Full High/Specialist set | 25–35% |
| Full top-tier specialist set | 35–50% |

If Kenshi's job formulas amplify or damp effective skill differently, the affix percentages must be adjusted to hit these gameplay targets.

## Controlled methodology

For every profession:

1. Use a clean fixture save and the same character, workstation/resource state and location for every sample.
2. Set or select base skill bands:
   - 10
   - 25
   - 50
   - 75
   - 90
3. Measure an unmodified baseline at each skill.
4. Apply controlled synthetic effective-stat bonuses:
   - +2%
   - +4%
   - +6%
   - +8%
   - +10%
   - +15%
   - +20%
   - +25%
   - +35%
   - +50%
5. Run the same job repeatedly.
6. Measure in-game elapsed time and completed output.
7. Run at least 5 repeats per point; use 10 if variance exceeds 3%.
8. Reset job/input/output state between repeats.
9. Calculate median, mean and standard deviation.
10. Convert each bonus into real throughput gain:
   `gain% = (baseline_time / modified_time - 1) * 100`
11. Choose tier ranges only after the real response curve is known.

## Profession benchmark jobs

Use the simplest deterministic job that directly exercises the relevant stat.

| Profession stat | Preferred benchmark |
|---|---|
| Farming | Repeated harvest of the same mature farm/crop type |
| Labouring | Mine a fixed quantity from the same resource node |
| Science | Research a fixed-progress technology segment |
| Engineering | Build/repair a fixed construction amount |
| Robotics | Robotics repair/crafting task with fixed inputs |
| Medic | Heal a controlled fixed injury amount |
| Weapon Smithing | Craft the same weapon recipe |
| Armour Smithing | Craft the same armour recipe |
| Crossbow Smithing | Craft the same crossbow/bolt recipe as appropriate |
| Cooking | Produce a fixed count of the same food recipe |
| Turrets | Controlled turret operation benchmark where measurable |
| Athletics | Fixed-distance run with same encumbrance/terrain |
| Swimming | Fixed-distance swim |
| Perception | Only if a deterministic gameplay outcome can be measured; otherwise balance by native stat-equivalence |
| Stealth | Controlled detection-distance/time benchmark |
| Lockpicking | Fixed lock level and repeated attempts/time |
| Assassination | Success-rate curve against fixed target |
| Thievery | Success-rate curve against fixed target/context |

For probability-based skills, use enough trials for stable estimates instead of completion-time samples.

## Full-set tests

After single-stat response curves are measured, test realistic profession loadouts.

### Farming
- Hoe
- Straw/work hat
- Work gloves
- Work boots
- Crop pack

### Labouring/mining
- Pickaxe
- Miner goggles/hat
- Work gloves
- Work boots
- Ore pack

### Research
- Research tool
- Lab coat
- Research goggles/visor
- Precision/work gloves
- Tech pack

### Engineering
- Engineering tool/tool belt
- Protective headwear
- Work gloves
- Work boots
- Construction pack

### Medical
- Medical tool
- Medical clothing
- Gloves
- Medical pack

### Smithing
- Appropriate smithing tool
- Apron/body gear
- Protective headwear
- Gloves
- Work boots

Each loadout must be tested at low, medium and high base skill.

## Progression sanity checks

For every profession, verify:

- Skill 75 + ordinary gear beats Skill 25 + top-tier gear.
- Skill 90 + ordinary gear is not made irrelevant by gear.
- Skill 50 + complete Shoddy gear is meaningfully but modestly faster than naked Skill 50.
- A top-tier single item does not equal a complete top-tier set.
- A best-roll lower-tier item can overlap a worst-roll next-tier item without regularly outperforming the entire next tier.
- Removing one item changes performance by roughly the contribution expected from that item.
- Backpack bonuses do not double-count profession-stat bonuses.

## Tier-fitting method

After collecting real response data, fit each profession separately if needed.

Example:

If Farming +6% effective skill produces only +1.5% harvest throughput, Farming affix percentages can be larger.

If Labouring +6% effective skill produces +8% mining throughput, Labouring affixes must be smaller.

Do **not** force one universal percentage table across every profession unless measurements show the response curves are sufficiently similar.

Recommended process:

1. Compute the real throughput response curve for each profession.
2. Set desired single-item throughput target by tier.
3. Numerically solve/lookup the effective-stat bonus needed for that target.
4. Set min/max affix values around that point.
5. Test complete sets.
6. Reduce individual ranges if stacking causes the full-set target to overshoot.
7. Repeat until progression targets are met.

## Balance test cases

These extend TEST_PLAN.md.

### Farming calibration
161. Baseline Farming 10 throughput.
162. Baseline Farming 25 throughput.
163. Baseline Farming 50 throughput.
164. Baseline Farming 75 throughput.
165. Baseline Farming 90 throughput.
166. Farming 50 response at +2/+4/+6/+8/+10%.
167. Farming 50 response at +15/+20/+25/+35/+50%.
168. Farming low-skill response curve at 25.
169. Farming high-skill response curve at 75/90.
170. Single Shoddy hoe min-roll throughput.
171. Single Shoddy hoe midpoint-roll throughput.
172. Single Shoddy hoe max-roll throughput.
173. Full Shoddy farming set throughput.
174. Full mid-tier farming set throughput.
175. Full top-tier farming set throughput.
176. Farming skill-vs-gear sanity comparisons.

### Labouring calibration
177. Labouring baseline at 10/25/50/75/90.
178. Labouring 50 full response sweep.
179. Single Shoddy pickaxe min/mid/max.
180. Full Shoddy mining set.
181. Full mid-tier mining set.
182. Full top-tier mining set.
183. Labouring skill-vs-gear sanity comparisons.

### Production professions
184. Science baseline/response sweep.
185. Engineering baseline/response sweep.
186. Robotics baseline/response sweep.
187. Cooking baseline/response sweep.
188. Weapon Smithing baseline/response sweep.
189. Armour Smithing baseline/response sweep.
190. Crossbow Smithing baseline/response sweep.
191. Medic controlled-healing response sweep.
192. Turret controlled response sweep where measurable.

### Utility skills
193. Athletics fixed-route baseline/response sweep.
194. Swimming fixed-route baseline/response sweep.
195. Stealth controlled detection benchmark.
196. Lockpicking probability/time benchmark.
197. Assassination probability benchmark.
198. Thievery probability benchmark.
199. Perception benchmark if a deterministic metric can be established.

### Cross-system and final fitting
200. Compare response curves across professions.
201. Determine whether universal tier ranges are acceptable.
202. Fit profession-specific multipliers if response curves differ materially.
203. Verify full-set target bands at Skill 25.
204. Verify full-set target bands at Skill 50.
205. Verify full-set target bands at Skill 75.
206. Verify Skill 75 ordinary > Skill 25 top-tier across each core profession.
207. Verify no complete top-tier set exceeds chosen maximum throughput target.
208. Verify adjacent tier roll overlap behaves as intended.
209. Re-run balance suite after any affix-range adjustment.
210. Freeze v1 balance constants only after all core professions satisfy target bands.

## Data capture format

Each measured run should record:

- test ID
- profession
- character
- base skill
- effective bonus %
- effective displayed/calculated skill
- gear pieces
- item tier/rolls
- job/recipe/farm/node
- input state
- output target
- game start timestamp
- game finish timestamp
- elapsed game seconds
- real elapsed seconds
- game speed
- result count
- notes
- pass/fail validity

Store results as CSV so they can be processed automatically.

## Automation integration

Once live installation/testing is authorized, add ProfessionGear-specific scenario commands rather than requiring Shay to time jobs manually: the ProfessionGear plugin can register them in the Kenshi Automation Harness at run time (`C:\KenshiModding\Kenshi-Automation-Harness`, `include/KenshiAutomationHarness.h`, `docs/EXTENDING.md`); they then show up in `stobe-auto help` and run through `stobe-auto <command>`.

Needed operations:
- load fixture
- select/identify worker
- set controlled skill value or select prepared worker
- give/equip exact test item
- force a specific test affix/roll
- clear profession gear
- seed workstation/farm/input state
- set game speed
- start job
- detect completion/output delta
- write result row
- reset fixture

The runner should automatically iterate skill/bonus matrices and emit CSV plus a summarized recommendation.

## Current state (2026-10-06)

All professions were measured in game (gates + matrices m28-m47; summary in `INGAME_STATUS.md` "Balance state");
Shay decisions D1-D8 applied. Open: D4 athletics feel and Shay's whole-picture balance decision.
