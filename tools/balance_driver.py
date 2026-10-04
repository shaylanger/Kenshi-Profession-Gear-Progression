#!/usr/bin/env python3
"""Balance measurement driver (TEST_PLAN Section L, rows 161-199; fit rows 200-210 via tools/analyze_balance.py).

gen: write one harness scenario file per profession (tests/ingame/full-base/pg-5x/8x-balance-*.txt). Every
     measured point ends with an `@echo BAL2,...` line. Measurement kinds:
       craft      crafts queued at a bench; result = queue drop in a fixed game-time window
       operate    production building / farm worker ticks: PG `pg_operate` output_progress in a window
       research   `research status` progress (0..1) of the queued tech in a window
       construct  harness `construct`: a fresh construction site per window, progress (0..1) in the window
       heal       harness `healtime ... wound <cut>`: bandaging per game second while treating a fixed wound
       move       harness `swimtime` on dry land (any axis): seconds for a fixed run, plus `runspeed`
       swim       harness `findwater` + `swimtime`: swim speed through deep water
       detect     harness `detecttime`: game seconds until an observer sees a sneaking character
                  (stealth: the sneaker's skill/gear varies; perception: the observer's)
       chance     harness `chance`: the game's own probability (lockpick, stealth KO, steal)
csv: turn the BAL2 (and old BAL) lines of runner .out files into the CSV tools/analyze_balance.py reads
     (test_id,profession,base_skill,effective_bonus_pct,elapsed_game_seconds,result_count,valid,gear_loadout,notes).
     Throughput = result_count / elapsed_game_seconds, so for every kind "higher = better for the worker":
       craft/operate/research/construct: amount done / window; heal: bandaging / seconds bandaging;
       move/swim: distance / seconds; detect (stealth): seconds unseen / 1; detect (perception): 1 / seconds;
       chance: chance / 1.
     gear_loadout lab50 (Labouring +50% on a non-Labouring job) is a control: written with bonus 0 and
     profession "<prof>_lab50ctl" so it never mixes with the real sweep.

Usage:
  python3 tools/balance_driver.py gen                 (rewrites every generated file)
  python3 tools/balance_driver.py list                (files, rows, kinds, window counts)
  python3 tools/balance_driver.py csv run.out... > balance.csv
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "tests", "ingame", "full-base")

# Fixture Testing-Save-Full-Base: squad Beaks + Avarek. The worker is Avarek, Beaks stays the selection.
WORKER = "Avarek"
OTHER = "Beaks"
GEAR_ITEM = "Iron Hat"       # given + worn by whoever's stat is varied; its affix is forced per point
NEEDS = ("ProfessionGearProgression.dll 7D80DBB3+ (D7A60E49 ok), harness 33087EDE+ (KAH 24: chance, detect, "
         "detecttime, healtime, findwater, swimtime, construct, construction); client kah.py from the harness "
         "repo (LONG_COMMANDS has the KAH 24 timers); set_test_mode.ps1 -Mode Forced -Rules InGameTest")

# Measurement grids.
BASE = 50
TIMED_SKILLS = [10, 90]                                  # no gear, one window each (curve end points)
TIMED_GEAR = [("none", None, 0), ("own25", "OWN", 25), ("own50", "OWN", 50), ("lab50", "labouring", 50)]
TIMED_REPEATS = 3
EVENT_SKILLS = [10, 90]
EVENT_GEAR = TIMED_GEAR
EVENT_REPEATS = 3
# Skill-vs-gear sanity points (rows 176, 183, 206): skill 75 without gear vs skill 25 with +50% gear.
SANITY = [(75, ("none", None, 0)), (25, ("own50", "OWN", 50))]
CHANCE_SKILLS = [10, 25, 50, 75, 90]
CHANCE_GEAR = [("own2", "OWN", 2), ("own5", "OWN", 5), ("own10", "OWN", 10), ("own25", "OWN", 25),
               ("own50", "OWN", 50), ("own100", "OWN", 100), ("lab50", "labouring", 50)]

# setstat names (harness) and pg stat names (pg_force_affix / pg_bonus).
P = [
    # ---- crafting benches (window = queue drop) ----
    dict(file="pg-52-balance-weapon-smithing.txt", rows="188", kind="craft", prof="weapon_smithing",
         setstat="weapon_smith", bench="Weapon Smith", item="Sickle", give=[("Iron Plates", 15)], window=10,
         prep=['research "Basic Weapon Smithing"', 'research "Basic Weapon Grades"', 'research "Utility Weapons"'],
         check=r'benches 400 crafts ~ Weapon Smith[^|]*Sickle'),
    dict(file="pg-53-balance-armour-smithing.txt", rows="189", kind="craft", prof="armour_smithing",
         setstat="armour_smith", bench="Clothing", item="Rag Shirt", give=[("584-gamedata.base", 15)], window=10,
         prep=['blueprint "Rag Shirt"'], check=r'benches 400 crafts ~ Clothing[^|]*Rag Shirt'),
    dict(file="pg-80-balance-crossbow-smithing.txt", rows="190", kind="craft", prof="crossbow_smithing",
         setstat="crossbow_smith", bench="Crossbow Crafting", item="Junkbow",
         give=[("Steel Bars", 40), ("Hinge", 10)], window=10,
         prep=['research "Crossbow Crafting"'], check=r'benches 400 crafts ~ Crossbow Crafting[^|]*Junkbow'),
    dict(file="pg-81-balance-robotics.txt", rows="186", kind="craft", prof="robotics", setstat="robotics",
         bench="Robotics Bench", item="Gears", give=[("Iron Plates", 15)], window=10,
         prep=['research "Robotics"'], check=r'benches 400 crafts ~ Robotics Bench[^|]*Gears'),
    dict(file="pg-82-balance-cooking.txt", rows="187", kind="craft", prof="cooking", setstat="cooking",
         bench="Cooking Stove", item="Dried Meat", give=[("Raw Meat", 15)], window=10,
         prep=[], check=r'benches 400 crafts ~ Cooking Stove[^|]*Dried Meat'),
    # ---- work over time ----
    dict(file="pg-90-balance-farming.txt", rows="161-176", kind="operate", prof="farming", setstat="farming",
         building="Wheat Farm L", fill=("Water", 40), window=30, extra_skills=[25, 75]),
    dict(file="pg-54-research.txt", rows="184", kind="research", prof="science", setstat="science",
         building="Research Bench", window=10,
         techs=["Research Bench II", "Research Bench III", "Stone Processing", "Crossbow Bolts", "Bonedog Breeding",
                "Hydroponics", "Advanced Cooking", "Iron Plates"]),
    dict(file="pg-83-balance-engineering.txt", rows="185", kind="construct", prof="engineering",
         setstat="engineering", building="Small Shack", window=5),
    # ---- timed events ----
    dict(file="pg-84-balance-medic.txt", rows="191", kind="heal", prof="medic", setstat="medic", cut=30),
    dict(file="pg-85-balance-athletics.txt", rows="193", kind="move", prof="athletics", setstat="athletics", dist=40),
    dict(file="pg-86-balance-swimming.txt", rows="194", kind="swim", prof="swimming", setstat="swimming", dist=30),
    dict(file="pg-87-balance-stealth.txt", rows="195", kind="detect", prof="stealth", setstat="stealth",
         side="sneaker", dist=20, timeout=120),
    dict(file="pg-89-balance-perception.txt", rows="199", kind="detect", prof="perception", setstat="perception",
         side="observer", dist=20, timeout=120),
    # ---- the game's own chances ----
    dict(file="pg-88-balance-chances.txt", rows="196-198", kind="chance", subs=[
        dict(row="196", prof="lockpicking", setstat="lockpicking", what='lockpick "Prisoner Cage"',
             key="lockpick_chance"),
        dict(row="197", prof="assassination", setstat="assassination", what="ko PGTarget", key="stealth_ko_chance"),
        dict(row="198", prof="thievery", setstat="thievery", what='steal PGTarget item "Iron Plates"',
             key="steal_chance"),
    ]),
]

NUM = r"([-\d.]+)"


def pgstat(prof, stat):
    return prof if stat == "OWN" else stat


def gear_lines(who, prof, gear):
    label, stat, pct = gear
    lines = ["pg_clear %s ~ cleared affixes" % who]
    if stat:
        s = pgstat(prof, stat)
        lines.append('pg_force_affix %s "%s" %s %d tier 6 ~ affixes=' % (who, GEAR_ITEM, s, pct))
        lines.append("pg_bonus %s %s ~ equipped_bonus=%d%% .*match=1" % (who, s, pct))
    return lines


def echo(row, prof, skill, gear, label, kind, window_s, a, b):
    bonus = gear[2] if gear[1] == "OWN" else 0
    return "@echo BAL2,%s,%s,%d,%d,%s,%s,%s,%s,%s,%s" % (row.split("-")[0], prof, skill, bonus, gear[0], label, kind,
                                                       window_s, a, b)


def points(skills, gears, repeats, base=BASE):
    """[(label, skill, gear)]: skills without gear, then repeats x gears at the base skill."""
    out, n = [], 0
    for s in skills:
        n += 1
        out.append(("w%d" % n, s, ("none", None, 0)))
    for _ in range(repeats):
        for g in gears:
            n += 1
            out.append(("w%d" % n, base, g))
    for skill, g in SANITY:  # rows 206/176/183: skill 75 ordinary vs skill 25 + strong gear
        n += 1
        out.append(("w%d" % n, skill, g))
    return out


def header(p, design, verify):
    return [
        "# id: PG-%s" % os.path.splitext(p["file"])[0],
        "# covers: PG %s (Section L balance sweep, generated by tools/balance_driver.py gen; edit the generator, "
        "not this file)" % p["rows"],
        "# fixture: Testing-Save-Full-Base (squad %s + %s)" % (OTHER, WORKER),
        "# reset: fresh",
        "# needs: " + NEEDS,
    ] + ["# design: " + design[0]] + ["#   " + d for d in design[1:]] + ["# verify: " + verify[0]] + \
        ["#   " + v for v in verify[1:]] + [
        "#   Data: python3 tools/balance_driver.py csv <out> > balance.csv; python3 tools/analyze_balance.py balance.csv",
    ]


def start(extra=()):
    return ["@wait-world", "@sleep 8", "speed 0",
            "protect %s on" % OTHER, "protect %s on" % WORKER,
            "hunger %s 280" % WORKER, "hunger %s 280" % OTHER,
            "select %s" % OTHER, "clearjobs %s" % WORKER] + list(extra)


def wear(who):
    return ['give %s "%s" 1' % (who, GEAR_ITEM), 'equip %s "%s"' % (who, GEAR_ITEM),
            'pg_info %s "%s" ~ equipped=1' % (who, GEAR_ITEM)]


def timed_points(p):
    skills = sorted(set(TIMED_SKILLS + p.get("extra_skills", [])))
    return points(skills, TIMED_GEAR, TIMED_REPEATS)


# ---------------------------------------------------------------- kinds
def gen_craft(p):
    w = p["window"]
    L = header(p, ["%d crafts of %s queued at the %s before every %d-game-minute window at speed 10;"
                   % (10, p["item"], p["bench"], w),
                   "skill %s without gear, then %d x (skill %d: none / +25%% / +50%% %s / +50%% Labouring control)."
                   % ("/".join(map(str, TIMED_SKILLS)), TIMED_REPEATS, BASE, p["prof"])],
               ["every window has a BAL2 line with two queue values and a drop > 0 at skill 50 (a drop of 10 =",
                "saturated: tell the PG agent to shorten the window). own > none = the bench reads the hooked stat."])
    L += start(p["prep"] + [p["check"]]) + wear(WORKER)
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ['give %s "%s" %d' % (WORKER, it, n) for it, n in p["give"]]
        L += ['craft %s "%s" at "%s" count 10 ~ queued' % (WORKER, p["item"], p["bench"]),
              '@set Q0 benches 400 ~ %s[^|]*queue=(\\d+)' % p["bench"],
              "speed 10", "@wait-game %d 900" % w, "speed 0",
              '@set Q1 benches 400 ~ %s[^|]*queue=(\\d+)' % p["bench"],
              echo(p["rows"], p["prof"], skill, gear, label, "craft", w * 60, "${Q0}", "${Q1}"),
              "hunger %s 280" % WORKER]
    return L + ["clearjobs %s" % WORKER]


def gen_operate(p):
    w, b = p["window"], p["building"]
    L = header(p, ["%s worked by %s (its own job); pg_operate output_progress per %d-game-minute window at speed 10"
                   % (b, WORKER, w),
                   "(JobOperateScaling on = the default build). Skills %s without gear, then %d x gear points at %d."
                   % ("/".join(map(str, sorted(set(TIMED_SKILLS + p.get("extra_skills", []))))), TIMED_REPEATS, BASE)],
               ["calls > 0 in every window; own50 ~ x1.5 of none at the same skill (pg-13 measured x1.50 on auto-home)."])
    L += start(['teleport %s building "%s" dist 6 radius 1000' % (WORKER, b),
                'building "%s" 1000' % b,
                'fill "%s" %s %d radius 1000' % (b, p["fill"][0], p["fill"][1]),
                'job %s "%s" radius 1000' % (WORKER, b),
                "pg_jobscale on ~ jobOperateScaling=1"]) + wear(WORKER)
    L += ["speed 10", "@wait-game 10 900",
          '@until 300 pg_operate "%s" radius 100 near %s ~ calls=[1-9]' % (b, WORKER), "speed 0"]
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ['fill "%s" %s %d radius 1000' % (b, p["fill"][0], p["fill"][1]),
              'pg_operate "%s" reset radius 100 near %s ~ \\(reset\\)' % (b, WORKER),
              "speed 10", "@wait-game %d 900" % w, "speed 0",
              '@set OP pg_operate "%s" radius 100 near %s ~ output_progress=%s' % (b, WORKER, NUM),
              echo(p["rows"], p["prof"], skill, gear, label, "operate", w * 60, "0", "${OP}"),
              "hunger %s 280" % WORKER]
    return L + ["clearjobs %s" % WORKER]


def gen_research(p):
    w = p["window"]
    L = header(p, ["%s researches at the %s (job); research status progress (0..1) of the first queued tech per"
                   % (WORKER, p["building"]),
                   "%d-game-minute window at speed 10. Several techs are queued (each start that the game refuses" % w,
                   "is one FAIL line, harmless): the first that starts is measured; progress resets to the next tech",
                   "when one completes (then that window's BAL2 is invalid: P1 < P0)."],
               ["progress rises in every window (else no power/bench level: research status desk_level/benches);",
                "own50 > none = research reads the hooked Science (ResearchBuilding::operate is not scaled by PG)."])
    L += start(["research status"] + ['research start "%s"' % t for t in p["techs"]] +
               ["research status ~ queue=[1-9]",
                'job %s "%s" radius 500' % (WORKER, p["building"])]) + wear(WORKER)
    L += ["speed 10", "@wait-game 5 600", "speed 0"]
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["@set P0 research status ~ progress=%s" % NUM,
              "speed 10", "@wait-game %d 900" % w, "speed 0",
              "@set P1 research status ~ progress=%s" % NUM,
              echo(p["rows"], p["prof"], skill, gear, label, "research", w * 60, "${P0}", "${P1}"),
              "hunger %s 280" % WORKER]
    return L + ["research status", "clearjobs %s" % WORKER]


def gen_construct(p):
    w, b = p["window"], p["building"]
    L = header(p, ["per window a fresh %s construction site (harness construct: unfinished, materials in, BUILD job)" % b,
                   "next to %s, built for %d game minutes at speed 10; result = construction progress (0..1)." % (WORKER, w),
                   "The site is removed after each window (unbuild)."],
               ["progress > 0 and < 1 in every window (1 = saturated: tell the PG agent to shorten the window or",
                "pick a bigger building; the construct reply shows build_hours). own50 > none = building reads the",
                "hooked Engineering."])
    L += start(["teleport %s %s dist 30" % (WORKER, OTHER)]) + wear(WORKER)
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ['@set P0 construct %s "%s" dist 12 ~ progress=%s' % (WORKER, b, NUM),
              "speed 10", "@wait-game %d 900" % w, "speed 0",
              '@set P1 construction "%s" ~ progress=%s' % (b, NUM),
              echo(p["rows"], p["prof"], skill, gear, label, "construct", w * 60, "${P0}", "${P1}"),
              "clearjobs %s" % WORKER,
              'unbuild "%s"' % b,
              "hunger %s 280" % WORKER]
    return L


def gen_heal(p):
    L = header(p, ["%s (medic, 5 Basic First Aid Kits) treats %s: per point healtime ... wound %d gives %s full"
                   % (WORKER, OTHER, p["cut"], OTHER),
                   "health, no bandages and the same three cuts, then times the first aid at speed 1. %s is NOT" % OTHER,
                   "protected while treated (protect heals wounds). Result = bandaging added per game second."],
               ["finished=1 in every point; own50 > none = first aid reads the hooked Medic."])
    L += start(['give %s "Basic First Aid Kit" 5' % WORKER, "teleport %s %s dist 4" % (WORKER, OTHER),
                "protect %s off" % OTHER]) + wear(WORKER)
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s %s dist 4" % (WORKER, OTHER), "speed 1",
              "@set H healtime %s %s wound %d timeout 600 ~ (seconds_bandaging=[\\d.]+ bandaging [\\d.]+ -> [\\d.]+)"
              % (WORKER, OTHER, p["cut"]),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "heal", 0, "${H}", "-"),
              'give %s "Basic First Aid Kit" 1' % WORKER,
              "hunger %s 280" % WORKER]
    return L + ["health %s 100" % OTHER, "protect %s on" % OTHER]


def gen_move(p):
    d = p["dist"]
    L = header(p, ["%s runs %d m along +x on open ground 150 m from %s (harness swimtime on land: any axis, seconds"
                   % (WORKER, d, OTHER),
                   "of game time), back to the start by teleport; also the game's movement speed (runspeed)."],
               ["every point arrives (no 'did not arrive'); speed rises with Athletics; own50 > none = movement",
                "reads the hooked Athletics."])
    L += start(["teleport %s %s dist 150" % (WORKER, OTHER), "water %s ~ water_level=none" % WORKER]) + wear(WORKER)
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s %s dist 150" % (WORKER, OTHER), "speed 1", "@sleep 2",
              "@set RS runspeed %s ~ movement_speed=%s" % (WORKER, NUM),
              "@set T swimtime %s %d +x run ~ (swam [\\d.]+ m seconds=[\\d.]+)" % (WORKER, d),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "move", 0, "${T}", "${RS}")]
    return L + ["teleport %s %s dist 3" % (WORKER, OTHER)]


def gen_swim(p):
    d = p["dist"]
    L = header(p, ["findwater (radius 3000, depth 1.5) gives the nearest deep water and its longest straight run;",
                   "%s is teleported there per point and swims %d m along that run (swimtime)." % (WORKER, d),
                   "Result = swim_speed (distance in deep water / seconds in deep water)."],
               ["findwater finds water with best_run >= %d (else: this fixture has no water within 3 km: tell the" % d,
                "PG agent which save has some); deep_fraction near 1; own50 > none = swimming reads the hooked stat."])
    L += start(["@set WX findwater %s radius 3000 depth 1.5 ~ water at ([-\\d.]+ [-\\d.]+ [-\\d.]+)" % WORKER,
                "@set WD findwater %s radius 3000 depth 1.5 ~ best=(\\S+)" % WORKER,
                "findwater %s radius 3000 depth 1.5 ~ best_run=([3-9]\\d|[1-9]\\d\\d)" % WORKER]) + wear(WORKER)
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s ${WX}" % WORKER, "speed 1", "@sleep 3",
              "water %s ~ water_level=deep" % WORKER,
              "@set T swimtime %s %d ${WD} ~ (deep_seconds=[\\d.]+ deep_dist=[\\d.]+)" % (WORKER, d),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "swim", 0, "${T}", "-"),
              "hunger %s 280" % WORKER]
    return L + ["teleport %s %s dist 3" % (WORKER, OTHER)]


def gen_detect(p):
    d, to = p["dist"], p["timeout"]
    sneaker_varies = p["side"] == "sneaker"
    who = WORKER if sneaker_varies else "PGWatch"
    L = header(p, ["a neutral observer (Hungry Bandit, Drifters, renamed PGWatch) stands %d m from %s, who sneaks"
                   % (d, WORKER),
                   "(detecttime: sneak on, the observer's earlier sighting reset). Result = game seconds until seen",
                   "(timeout %d s). The %s's skill/gear varies (%s)." % (to, p["side"], p["prof"]),
                   "Both are put back to their places before every point; the observer is killed at the end."],
               ["seen=1 in most points (seen=0 everywhere: this observer never notices a sneaker, send the .out to",
                "the PG agent); stealth: seconds_to_seen grows with Stealth; perception: it shrinks with Perception."])
    L += start(["teleport %s %s dist 60" % (WORKER, OTHER),
                "setstat %s stealth 30" % WORKER,
                "@set OBS spawn \"Hungry Bandit\" Drifters near %s dist %d count 1 ~ spawned 1/1 [^:]+: .+? (#\\d+(?:/\\d+)?)"
                % (WORKER, d),
                "setname ${OBS} PGWatch",
                "relation PGWatch 0",
                "setstat PGWatch perception 30",
                "where PGWatch"]) + wear(who)
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: %s %d, gear %s ---" % (label, p["prof"], skill, gear[0]),
              "setstat %s %s %d" % (who, p["setstat"], skill)] + gear_lines(who, p["prof"], gear)
        L += ["stealth %s off" % WORKER, "teleport %s %s dist 60" % (WORKER, OTHER),
              "teleport PGWatch %s dist %d" % (WORKER, d), "speed 1", "@sleep 3",
              "@set T detecttime %s PGWatch timeout %d ~ (seen=\\d seconds_to_seen=\\S+)" % (WORKER, to),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "detect_" + p["side"], to, "${T}", "-")]
    return L + ["stealth %s off" % WORKER, "teleport PGWatch %s dist 300" % WORKER, "kill PGWatch",
                "teleport %s %s dist 3" % (WORKER, OTHER)]


def gen_chance(p):
    L = header(p, ["the game's own chance functions read by the harness `chance` command (no randomness: one read per",
                   "point, the skill-50 no-gear point read twice): lockpick a Prisoner Cage (196), stealth KO (197) and",
                   "steal Iron Plates (198) against a neutral dummy (Hungry Bandit, Drifters, renamed PGTarget) 3 m",
                   "away, %s sneaking; the whole file runs paused (speed 0), so nobody moves or fights. Skills %s"
                   " without gear, then gear %s at 50."
                   % (WORKER, "/".join(map(str, CHANCE_SKILLS)), "/".join(g[0] for g in CHANCE_GEAR))],
               ["chance rises with the skill; ownN > none = the chance reads the hooked stat; lab50 = none."])
    L += start(["teleport %s building \"Prisoner Cage\" dist 4 radius 1000" % WORKER,
                "@set TGT spawn \"Hungry Bandit\" Drifters near %s dist 40 count 1 ~ spawned 1/1 [^:]+: .+? (#\\d+(?:/\\d+)?)"
                % WORKER,
                "setname ${TGT} PGTarget", "relation PGTarget 0",
                'give PGTarget "Iron Plates" 2',
                "teleport PGTarget %s dist 3" % WORKER,
                "stealth %s on" % WORKER]) + wear(WORKER)
    for sub in p["subs"]:
        prof = sub["prof"]
        pts = [("s%d" % s, s, ("none", None, 0)) for s in CHANCE_SKILLS] + [("s50b", BASE, ("none", None, 0))] + \
              [(g[0], BASE, g) for g in CHANCE_GEAR] +               [("s25_own50", 25, ("own50", "OWN", 50)), ("s25_own100", 25, ("own100", "OWN", 100))]
        for label, skill, gear in pts:
            L += ["# --- %s %s: skill %d, gear %s ---" % (sub["row"], prof, skill, gear[0]),
                  "setstat %s %s %d" % (WORKER, sub["setstat"], skill)] + gear_lines(WORKER, prof, gear)
            L += ["teleport PGTarget %s dist 3" % WORKER,
                  "@set C chance %s %s ~ %s=%s" % (WORKER, sub["what"], sub["key"], NUM),
                  echo(sub["row"], prof, skill, gear, label, "chance", 0, "${C}", "-")]
    return L + ["stealth %s off" % WORKER, "teleport PGTarget %s dist 300" % WORKER, "kill PGTarget"]


GEN = dict(craft=gen_craft, operate=gen_operate, research=gen_research, construct=gen_construct, heal=gen_heal,
           move=gen_move, swim=gen_swim, detect=gen_detect, chance=gen_chance)


def gen():
    os.makedirs(OUT_DIR, exist_ok=True)
    for p in P:
        L = GEN[p["kind"]](p)
        path = os.path.join(OUT_DIR, p["file"])
        with open(path, "w", newline="\n") as fh:
            fh.write("\n".join(L) + "\n")
        n = sum(1 for x in L if x.startswith("@echo BAL2"))
        print("wrote %-42s rows %-8s kind %-9s points %d" % (p["file"], p["rows"], p["kind"], n))


# ---------------------------------------------------------------- csv
def f(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return None


def measure(kind, window_s, a, b):
    """(result_count, elapsed_game_seconds, valid, note) so that count/elapsed is 'higher = better'."""
    if kind == "craft":
        q0, q1 = f(a), f(b)
        if q0 is None or q1 is None:
            return 0, window_s, 0, "missing queue"
        done = q0 - q1
        return done, window_s, int(done > 0), "saturated" if done >= 10 else ""
    if kind in ("operate", "research", "construct"):
        p0, p1 = f(a), f(b)
        if p0 is None or p1 is None:
            return 0, window_s, 0, "missing progress"
        done = p1 - p0
        note = "saturated" if kind == "construct" and p1 >= 0.999 else ""
        return done * 1000.0, window_s, int(done > 0 and not note), note  # x1000: per-mille units
    if kind == "heal":
        m = re.search(r"seconds_bandaging=([\d.]+) bandaging ([\d.]+) -> ([\d.]+)", a or "")
        if not m:
            return 0, 1, 0, "no healtime reply"
        secs, b0, b1 = map(float, m.groups())
        return b1 - b0, max(secs, 0.01), int(secs > 0 and b1 > b0), ""
    if kind == "move":
        m = re.search(r"swam ([\d.]+) m seconds=([\d.]+)", a or "")
        if not m:
            return 0, 1, 0, "no swimtime reply"
        dist, secs = map(float, m.groups())
        return dist, max(secs, 0.01), int(secs > 0), "runspeed=%s" % b
    if kind == "swim":
        m = re.search(r"deep_seconds=([\d.]+) deep_dist=([\d.]+)", a or "")
        if not m:
            return 0, 1, 0, "no swimtime reply"
        secs, dist = map(float, m.groups())
        return dist, max(secs, 0.01), int(secs > 0 and dist > 0), ""
    if kind.startswith("detect"):
        m = re.search(r"seen=(\d) seconds_to_seen=(\S+)", a or "")
        if not m:
            return 0, 1, 0, "no detecttime reply"
        seen, secs = m.group(1) == "1", f(m.group(2))
        if kind == "detect_sneaker":   # longer unseen = better: censored at the timeout when never seen
            return (secs if seen else window_s), 1, 1, "" if seen else "censored(not seen)"
        if not seen:
            return 0, 1, 0, "not seen"
        return 1, max(secs, 0.01), 1, ""
    if kind == "chance":
        c = f(a)
        if c is None:
            return 0, 1, 0, "no chance"
        return c, 1, int(c > 0), ""
    return 0, 1, 0, "unknown kind " + kind


def to_csv(paths):
    print("test_id,profession,base_skill,effective_bonus_pct,elapsed_game_seconds,result_count,valid,gear_loadout,notes")
    for path in paths:
        base = os.path.basename(path)
        for line in open(path, encoding="utf-8", errors="replace"):
            m = re.search(r"=> BAL2,([^,]*),([^,]*),(\d+),(\d+),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)$", line)
            if m:
                tid, prof, skill, bonus, gear, label, kind, win, a, b = m.groups()
                if "${" in a:
                    a = ""
                if "${" in b:
                    b = ""
                count, secs, ok, note = measure(kind, f(win) or 0, a.strip(), b.strip())
                if gear == "lab50":
                    prof, bonus = prof + "_lab50ctl", "0"
                print("%s,%s,%s,%s,%s,%s,%d,%s,%s" % (tid, prof, skill, bonus, secs, count, ok, gear,
                                                     "%s %s %s %s" % (label, kind, note, base)))
                continue
            # pg-14 labouring curve (rows 177-183, 30 game minutes per window): "skill 50 +25%: ... output_progress=x"
            m = re.search(r"=> skill (\d+) (no gear|\+(\d+)%|set 25\+25): .*output_progress=([-\d.]+)", line)
            if m:
                skill, what, pct, prog = m.groups()
                bonus = 50 if what.startswith("set") else int(pct or 0)
                gear = "none" if what == "no gear" else ("set25+25" if what.startswith("set") else "own%d" % bonus)
                print("177,labouring,%s,%d,1800,%s,1,%s,pg-14 %s" % (skill, bonus, float(prog) * 1000.0, gear, base))
                continue
            m = re.search(r"=> BAL,([^,]+),([^,]+),(\d+),(\d+),(\d+),(\d*),(\d*),([^,]+),(\S+)", line)
            if m:  # the old pg-52/53 format
                tid, prof, skill, bonus, secs, q0, q1, gear, label = m.groups()
                count, _, ok, note = measure("craft", float(secs), q0, q1)
                if gear == "lab50":
                    prof, bonus = prof + "_lab50ctl", "0"
                print("%s,%s,%s,%s,%s,%s,%d,%s,%s" % (tid, prof, skill, bonus, secs, count, ok, gear,
                                                     "%s craft %s %s" % (label, note, base)))


def list_files():
    for p in P:
        print("%-42s rows %-8s kind %s" % (p["file"], p["rows"], p["kind"]))


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "gen":
        gen()
    elif len(sys.argv) >= 2 and sys.argv[1] == "list":
        list_files()
    elif len(sys.argv) >= 3 and sys.argv[1] == "csv":
        to_csv(sys.argv[2:])
    else:
        print(__doc__)
        sys.exit(2)
