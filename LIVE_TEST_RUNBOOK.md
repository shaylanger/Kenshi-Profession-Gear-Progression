# Profession Gear Progression — User Live-Test Runbook

This is the document Shay should follow after the mod is installed. The 306-row `TEST_PLAN.md` remains the technical master plan; this runbook translates it into concrete player actions.

## Before launch

1. Use a **new test game or copied test save**, never the main save.
2. Install the verified package with:
   `powershell -ExecutionPolicy Bypass -File .\install_test_build.ps1 -Install`
3. In the Kenshi launcher, enable **Profession Gear Progression**.
4. Keep RE_Kenshi/KenshiLib enabled.
5. For the first smoke run, enable forced test mode:
   `powershell -ExecutionPolicy Bypass -File .\set_test_mode.ps1 -Mode Forced`
6. Launch Kenshi.

After each phase below, tell ChatGPT **"Phase N done"**. The agent can inspect the logs/sidecar and mark the AUTO rows. Only the explicitly visual/game-feel checks require Shay's judgment.

## Phase 1 — startup

Player actions:
- Load the test world.
- Pause/unpause.
- Run normal speed, then high speed.
- Open inventory and several item tooltips.
- Make a manual save.
- Exit normally.

Pass from Shay:
- no crash/freeze,
- game feels normal.

Then report: **Phase 1 done**.

Covers startup/hook tests 31–45 plus packaging/startup hardening 273–290.

## Phase 2 — equip-only + tooltip

Use any eligible items you can obtain, ideally:
- Straw Hat / workwear,
- Wooden Sandals / travel footwear,
- goggles,
- backpack,
- profession-named mod tool if available.

For each useful affixed item:
1. Hover it and confirm the Profession Gear lines appear once.
2. Carry it without equipping it.
3. Note the relevant character skill/stat.
4. Equip it.
5. Note the same skill/stat again.
6. Unequip it.
7. Confirm the stat returns to baseline.
8. Move it between characters/inventories and back.
9. Save/reload and hover it again.

Pass from Shay:
- tooltip readable,
- no duplicate Profession Gear section,
- carried-only gear does not help,
- equipped gear does,
- unequip removes the bonus,
- same item keeps the same roll.

Then report: **Phase 2 done**.

Covers 46–84, 123–141, 214–216, 251–257, 273–300.

## Phase 3 — crafting

Craft several copies of any eligible item available in the current mod list.

Player actions:
- craft at least two same-quality eligible items,
- if possible craft items at different qualities / a critical-success result,
- inspect each tooltip,
- equip a crafted item,
- save/reload.

Pass from Shay:
- crafted copies can have different valid rolls,
- final Kenshi quality corresponds to the ProfessionGear tier,
- crafted item works immediately,
- save/reload keeps its roll.

Then report: **Phase 3 done**.

Covers 85–92 and 306.

## Phase 4 — NPC context

Interact with/inspect or loot a varied sample:
- farmer,
- miner/labourer,
- researcher/tech NPC,
- engineer,
- medic,
- smith,
- cook,
- slave/very poor NPC,
- ordinary soldier,
- named/unique NPC.

Kill/loot at least one eligible NPC if convenient, then save/reload.

Pass from Shay:
- profession gear looks contextually sensible,
- slaves/poor NPCs do not look overloaded with special gear,
- ordinary soldiers do not get nonsense profession rolls,
- explicitly unique/special gear is untouched.

Then report: **Phase 4 done**.

Covers 93–108 and 228–235.

## Phase 5 — vendors

Visit several relevant shops and inspect stock:
- armour/clothing,
- travel/backpack,
- medical,
- tech/research,
- general/trade vendors.

Buy at least one ProfessionGear item if one appears. Equip/unequip it and save/reload.

Pass from Shay:
- shop browsing can surface coherent profession/utility gear,
- no obviously nonsensical stat/item pairing.

Then report: **Phase 5 done**.

Covers 211–227 and 237–240.

## Phase 6 — exploration/world loot

Open several ruin/chest/container loot sources.

Player actions:
- inspect/pick up eligible clothing, goggles, packs or profession-like weapons,
- use different player characters if convenient,
- drop/pick up an affixed item,
- save/reload.

Pass from Shay:
- exploration can produce profession gear,
- loot does not obviously adapt to the profession of the character picking it up,
- repeated pickup does not reroll an item.

Then report: **Phase 6 done**.

Covers 241–250.

## Phase 7 — backpacks / hauling

Test any available specialist-like pack:
- ore/mining,
- crop/farm,
- construction,
- medical,
- tech/research,
- trade,
- hauling/cargo.

Compare inventory weight while:
1. pack is not equipped,
2. pack is equipped,
3. matching cargo is added,
4. unrelated cargo is added.

Also try a normal generic backpack.

Pass from Shay:
- extra ProfessionGear weight effect only works while equipped,
- specialist packs help matching cargo only,
- hauling pack helps cargo generally,
- generic backpack does not receive specialist weight reduction,
- vanilla inventory behavior still looks normal.

Then report: **Phase 7 done**.

Covers 109–122 and 291–305.

## Phase 8 — real job-path proof

This phase proves the most important technical assumption.

For available professions, compare baseline versus equipped ProfessionGear:
- Farming,
- Labouring/mining,
- Science,
- Engineering,
- Robotics,
- Cooking,
- Medic,
- Weapon/Armour/Crossbow smithing,
- Turrets.

Use the same character/job conditions where practical.

Pass criterion:
- the real job outcome or throughput changes consistently with the effective stat.

Then report: **Phase 8 done**.

Do not do final balance tuning until this phase proves the live stat paths.

## Phase 9 — utility proof

Test available:
- Running Shoes / Wooden Sandals / scout gear,
- swim/diving gear if installed,
- stealth/ninja gear,
- assassin gear,
- thief gear,
- generic/hauling backpacks.

Check that the relevant utility stat changes only while equipped and that gameplay changes in the expected direction where observable.

This phase decides whether Athletics and pack-weight support are enough or whether direct native movement/encumbrance hooks should be added.

Then report: **Phase 9 done**.

Covers 193–199 and 254–257.

## Phase 10 — normal rarity/distribution

Exit Kenshi and restore normal config:
`powershell -ExecutionPolicy Bypass -File .\set_test_mode.ps1 -Mode Normal`

Relaunch and play/inspect vendors, loot and NPCs normally.

This phase measures whether ProfessionGear is:
- common enough to discover,
- rare enough to feel meaningful,
- broadly available across professions,
- not producing nonsense.

Then report: **Phase 10 done**.

## Phase 11 — compatibility

Run the same test save with:
1. ProfessionGear + STOBE,
2. ProfessionGear + KenshiFP,
3. ProfessionGear + STOBE + KenshiFP,
4. your normal installed mod stack including UWE/GenMod.

Normal play actions:
- inventory/tooltips,
- equip/unequip,
- NPC conversations/gameplay,
- crafting,
- save/reload,
- high game speed.

Report any crash, duplicate tooltip, missing voice/FP behavior, inventory stall, or strange stat behavior.

Then report: **Phase 11 done**.

## Phase 12 — balance calibration

Only after Phases 1–11 pass, follow `BALANCE_TEST_PLAN.md`.

This is where provisional percentages, full-set strength and source frequencies are finalized from measured gameplay.

## If anything fails

Stop that phase and tell ChatGPT:
- **which phase**,
- what you did immediately before the problem,
- what you saw.

Do not delete the sidecar/log. The agent can collect and inspect the current test artifacts.

## Optional artifact collection

At any point:
`powershell -ExecutionPolicy Bypass -File .\collect_test_artifacts.ps1 -Label phase1`

Replace `phase1` with the relevant phase label.

## What Shay does vs what the agent does

Shay:
- installs/enables the mod,
- performs the gameplay actions above,
- judges tooltip readability/game feel,
- reports when each phase is complete or something breaks.

Agent:
- reads ProfessionGear logs and sidecar,
- checks exact roll/source/classification behavior,
- marks AUTO rows,
- investigates failures,
- updates regressions/code,
- analyzes balance data.

You do **not** need to manually execute or score all 306 rows one by one.
