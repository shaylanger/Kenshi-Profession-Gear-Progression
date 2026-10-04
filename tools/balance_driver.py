#!/usr/bin/env python3
"""Balance measurement driver (TEST_PLAN Section L, rows 161-199; fit rows 200-210 via tools/analyze_balance.py).

gen: write one harness scenario file per profession (tests/ingame/full-base/pg-5x/8x-balance-*.txt). Every
     measured point ends with an `@echo BAL2,...` line. Measurement kinds:
       craft      crafts queued at a bench; result = queue drop in a fixed game-time window
       operate    production building / farm worker ticks: PG `pg_operate` output_progress in a window
       research   `research status` progress (0..1) of the queued tech in a window
       construct  harness `construct`: a fresh construction site per window, progress (material units, Small Shack 0..7) in the window
       heal       harness `healtime ... wound <cut>`: bandaging per game second while treating a fixed wound
       move       harness `swimtime` on dry land (any axis): seconds for a fixed run, plus `runspeed`
       swim       harness `findwater` + `swimtime`: swim speed through deep water
       detect     harness `detecttime`: game seconds until an observer sees a sneaking character
                  (stealth: the sneaker's skill/gear varies; perception: the observer's)
       chance     harness `chance`: the game's own probability (lockpick, stealth KO, steal)
csv: turn the BAL2 (and old BAL) lines of runner .out files into the CSV tools/analyze_balance.py reads
     (test_id,profession,base_skill,effective_bonus_pct,elapsed_game_seconds,result_count,valid,gear_loadout,notes,
     setup,evidence,provenance): setup = READY/POST checks (ready=ok, or ready=FAIL with the failed check in
     notes), evidence = .out line + raw captures, provenance = the runner's PROV line (DLL hashes, PG ini/rules,
     fixture, scenario hash). Skill and gear have their own columns.
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

# Fixture Testing-Save-Full-Base: squad Beaks + Avarek. The worker is Avarek and stays the selection.
WORKER = "Avarek"
OTHER = "Beaks"
GEAR_ITEM = "Iron Hat"       # given + worn by whoever's stat is varied; its affix is forced per point
NEEDS = ("ProfessionGearProgression.dll 36D7474D+ (pg_statprobe), harness c5a5c88+ (give artifacts, research start any; 90a0e33: teleport moved=/bed, speed hold) (KAH 24: chance, detect, "
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
         setstat="weapon_smith", bench="Weapon Smith", item="Sickle", give=[("Steel Bars", 10), ("Fabrics", 10)],
         window=20, prep=['research "Basic Weapon Smithing"', 'research "Basic Weapon Grades"',
                          'research "Utility Weapons"', 'research "Katanas"', 'research "Basic Weapon Grades 3"'],
         check=r'benches 60 crafts ~ Weapon Smith[^|]*Sickle'),
    dict(file="pg-53-balance-armour-smithing.txt", rows="189", kind="craft", prof="armour_smithing",
         setstat="armour_smith", bench="Clothing", item="Rag Shirt", give=[("Fabrics", 10)], window=20,
         prep=['blueprint "Rag Shirt"'], check=r'benches 60 crafts ~ Clothing[^|]*Rag Shirt'),
    dict(file="pg-80-balance-crossbow-smithing.txt", rows="190", kind="craft", prof="crossbow_smithing",
         setstat="crossbow_smith", bench="Crossbow Crafting", item="Junkbow",
         give=[("Steel Bars", 10), ("Hinge", 10)], window=20,
         prep=['research "Crossbow Crafting"'], check=r'benches 60 crafts ~ Crossbow Crafting[^|]*Junkbow'),
    dict(file="pg-81-balance-robotics.txt", rows="186", kind="craft", prof="robotics", setstat="robotics",
         bench="Robotics Bench", item="Gears", give=[("Iron Plates", 15)], window=20,
         prep=['research "Robotics"'], check=r'benches 60 crafts ~ Robotics Bench[^|]*Gears'),
    dict(file="pg-82-balance-cooking.txt", rows="187", kind="craft", prof="cooking", setstat="cooking",
         bench="Cooking Stove", item="Dried Meat", give=[("Raw Meat", 15)], window=20,
         # Full-Base has no Cooking Stove (the Bread Oven runs without an operator): build one (m19-4080)
         prep=['teleport %s %s dist 30' % (WORKER, OTHER), 'build "Cooking Stove" near %s dist 10' % WORKER],
         check=r'benches 60 crafts ~ Cooking Stove[^|]*Dried Meat'),
    # ---- work over time ----
    dict(file="pg-90-balance-farming.txt", rows="161-176", kind="operate", prof="farming", setstat="farming",
         building="Wheat Farm L", fill=("Water", 40), window=30, extra_skills=[25, 75]),
    dict(file="pg-54-research.txt", rows="184", kind="research", prof="science", setstat="science",
         building="Research Bench", window=10,
         ),
    dict(file="pg-83-balance-engineering.txt", rows="185", kind="construct", prof="engineering",
         setstat="engineering", building="Small Shack", window=5),
    # ---- timed events ----
    dict(file="pg-84-balance-medic.txt", rows="191", kind="heal", prof="medic", setstat="medic", cut=30),
    dict(file="pg-85-balance-athletics.txt", rows="193", kind="move", prof="athletics", setstat="athletics", dist=40),
    dict(file="pg-86-balance-swimming.txt", rows="194", kind="swim", prof="swimming", setstat="swimming", dist=60),
    dict(file="pg-87-balance-stealth.txt", rows="195", kind="detect", prof="stealth", setstat="stealth",
         side="sneaker", dist=20, timeout=120),
    dict(file="pg-89-balance-perception.txt", rows="199", kind="detect", prof="perception", setstat="perception",
         side="observer", dist=20, timeout=120),
    # ---- the game's own chances ----
    # A player-owned/unoccupied cage can report zero at every skill. Compare a real
    # locked shackle with an explicit foreign owner before treating row 196 as no response.
    dict(file="pg-92-balance-owned-lock.txt", rows="196", kind="chance", owned_lock=True, subs=[
        dict(row="196", prof="lockpicking", setstat="lockpicking",
             what="lockpick PGTarget", key="lockpick_chance"),
    ]),
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
    # Avarek is often in bed in this save (in_bed_or_cage=1): teleport then silently does nothing (m19-4080)
    return ["@wait-world", "@sleep 8", "speed 0", "wake %s" % WORKER, "speed 1", "@sleep 4", "speed 0",
            "protect %s on" % OTHER, "protect %s on" % WORKER,
            "hunger %s 280" % WORKER, "hunger %s 280" % OTHER,
            # select the worker: an unselected Avarek walks into the base's Bed and lies there (m19-4080)
            "select %s" % WORKER, "clearjobs %s" % WORKER, "pg_statprobe on %s" % WORKER] + list(extra)


def wear(who):
    return ['give %s "%s" 1' % (who, GEAR_ITEM), 'equip %s "%s"' % (who, GEAR_ITEM),
            'pg_info %s "%s" ~ equipped=1' % (who, GEAR_ITEM)]


def ready(*checks):
    """Bounded readiness block before a measurement window (Shay 2026-10-04): every check is one command with a
    ~ assertion; a FAIL between "@echo READY" and the point's BAL2 makes that point invalid with the failed check as
    its reason (csv), so a worker who is asleep / off the bench / without materials, research, power or job is
    never fitted as a slow worker."""
    return ["@echo READY"] + list(checks)


def post(*checks):
    """Checks right after the window (worker still at the bench / still researching): same rule as ready()."""
    return ["@echo POST"] + list(checks)


AWAKE = r"where %s ~ ^(?!.*\b(KO|DEAD)\b)" % WORKER
HASJOB = r"jobs %s ~ jobs=[1-9]" % WORKER


def powered(b):
    # m22-4080: crafting/research stopped after the first windows with the worker at the bench (job kept):
    # power is held constant (harness `power ... supply`, c5a5c88+ also answers ok for benches that use none)
    # and checked before every window
    return r'building "%s" 60 ~ power_on=1 .*broken=0 .*(wants=0(\.0+)?\s|supplied=1)' % b


def bench_has(bench, item):
    # the bench's own entry only: " || <bench> ... | in1 6x3 limit=1: [Steel Bars x7 @..]"; '||' separates benches.
    # m23-4080: an INPUT section (in1, in2, ...), never "out": materials in out don't feed the craft (stalled at 2.1%)
    return r"benches 60 ~ %s[^|]*(?:\|[^|]+)*?\| in\d[^|]*\[%s x[1-9]" % (bench, item)


def timed_points(p):
    skills = sorted(set(TIMED_SKILLS + p.get("extra_skills", [])))
    return points(skills, TIMED_GEAR, TIMED_REPEATS)


# ---------------------------------------------------------------- kinds
def gen_craft(p):
    w, bench = p["window"], p["bench"]
    sp = p.get("speed", 20)
    L = header(p, ["%s works the %s (real craft job, %s queued 5 more per point); the bench input is refilled"
                   % (WORKER, bench, p["item"]),
                   "with %s before every %d-game-minute window at speed %d. Result = items made in the window:"
                   % (", ".join(it for it, _ in p["give"]), w, sp),
                   "queue drop + progress of the first queued item (benches prints it with 4 decimals, harness",
                   "after 33087EDE). Skills %s without gear, then %d x (skill %d: none / +25%% / +50%% %s / +50%%"
                   % ("/".join(map(str, TIMED_SKILLS)), TIMED_REPEATS, BASE, p["prof"]),
                   "Labouring control), then the sanity pair."],
               ["every window has a BAL2 line with Q/F values on both sides and made > 0 at skill 50;",
                "own > none = the bench reads the hooked stat."])
    q = '@set Q%d benches 60 ~ ' + bench + r'[^|]*queue=(\d+)'
    fpat = '@set F%d benches 60 ~ ' + bench + r'[^|]*queue=\d+ \(first: [^|]*? ([\d.]+)%%\)'
    L += start(p["prep"] + ["speed 1", "@sleep 2", "speed 0",
                            'teleport %s building "%s" dist 4 radius 1500' % (WORKER, bench),
                            "teleport %s %s dist 8" % (OTHER, WORKER), p["check"],
                            'power "%s" supply radius 60' % bench]) + wear(WORKER)
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        # m21-4080: crafting stalled after the first windows (worker off the bench, benches 60 found none):
        # put him back at the bench each point and log where he is / his jobs after the window.
        L += ['@until 30 teleport %s building "%s" dist 4 radius 1500 ~ moved=1' % (WORKER, bench)]
        # m23-4080 probe: bench input sections accept nothing until a craft is queued -> craft first, then fill
        L += ['craft %s "%s" at "%s" count 5 ~ queued' % (WORKER, p["item"], bench)]
        L += ['fill "%s" "%s" %d radius 60' % (bench, it, n) for it, n in p["give"]]
        # a "fill ... nothing fitted" top-up is not a failure by itself: the real bench state decides
        L += ready(AWAKE, *[bench_has(bench, it) for it, _ in p["give"]] +
                   [r"benches 8 ~ %s[^|]*queue=[1-9]" % bench, HASJOB, powered(bench)])
        L += [q % 0, fpat % 0,
              "speed %d" % sp, "@wait-game %d 900" % w, "speed 0",
              "where %s" % WORKER, "jobs %s" % WORKER]
        # m22-4080 pg-52 w10: he walked 500 m away during the window (benches 60 then found another bench)
        L += post(AWAKE, r"benches 8 ~ %s" % bench)
        L += [q % 1, fpat % 1,
              echo(p["rows"], p["prof"], skill, gear, label, "craft", w * 60, "${Q0}/${F0}", "${Q1}/${F1}"),
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
        L += ['fill "%s" %s %d radius 1000' % (b, p["fill"][0], p["fill"][1])]
        L += ready(AWAKE, HASJOB)
        L += ['pg_operate "%s" reset radius 100 near %s ~ \\(reset\\)' % (b, WORKER),
              "speed 10", "@wait-game %d 900" % w, "speed 0",
              '@set OP pg_operate "%s" radius 100 near %s ~ output_progress=%s' % (b, WORKER, NUM),
              echo(p["rows"], p["prof"], skill, gear, label, "operate", w * 60, "0", "${OP}"),
              "hunger %s 280" % WORKER]
    return L + ["clearjobs %s" % WORKER]


def gen_research(p):
    w = p["window"]
    L = header(p, ["%s researches at the %s (job); research status progress (0..1) of the first queued tech per"
                   % (WORKER, p["building"]),
                   "%d-game-minute window at speed 10. research start any 3 queues up to 3 techs the game would" % w,
                   "start now (longest first); the first is measured; progress resets to the next tech",
                   "when one completes (then that window's BAL2 is invalid: P1 < P0)."],
               ["progress rises in every window (else no power/bench level: research status desk_level/benches);",
                "own50 > none = research reads the hooked Science (ResearchBuilding::operate is not scaled by PG)."])
    # Full-Base: the bench is ~520 m from the squad and research costs Books (can_pay=0 without, m21-4080)
    # m23-4080: the research cost item is ITEM "Book" (`find item book` -> [Book]); "Books" = no data named. Hand-picked tech names were
    # finished / missing / unpayable in this save, so the harness picks startable techs (longest first)
    L += start(['@until 30 teleport %s building "%s" dist 4 radius 1500 ~ moved=1' % (WORKER, p["building"]),
                'give %s "Book" 40 ~ got [1-9]' % WORKER, "research status",
                'power "%s" supply radius 60' % p["building"],
                "research start any 3 ~ started=[1-9]",
                "research status ~ queue=[1-9]",
                'job %s "%s" radius 60' % (WORKER, p["building"])]) + wear(WORKER)
    L += ["speed 10", "@wait-game 5 600", "speed 0"]
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ['@until 30 teleport %s building "%s" dist 4 radius 1500 ~ moved=1' % (WORKER, p["building"])]
        L += ready(AWAKE, HASJOB, r"research status ~ queue=[1-9]", r"research status ~ ^(?!.*power_off)", powered(p["building"]))
        L += ["@set P0 research status ~ progress=%s" % NUM,
              "speed 10", "@wait-game %d 900" % w, "speed 0"]
        L += post(r"research status ~ researchers=[1-9]")
        L += ["@set P1 research status ~ progress=%s" % NUM,
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
              '@set P1 construction "%s" ~ progress=%s complete=0' % (b, NUM),
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
    # m22-4080: still flat ~4.3/s at Medic 10..90 after Beaks' kit was dropped, first bandage after 0.1 s:
    # control point with NO kit on the worker first. If the patient still gets bandaged, someone else treats
    # him and every heal point of this file is invalid (csv: heal_control).
    # m23-4080: give 1 + drop the whole stack (a plain drop FAILs when the kit is absent and voided the control);
    # healtime answers an error when nothing finished, so the control is an expected-error step (!) that must show
    # finished=0 and no bandaging. If someone bandages the patient the step FAILs and every heal point is invalid.
    nokit = sum([['give %s "%s First Aid Kit" 1' % (who, k), 'drop %s "%s First Aid Kit"' % (who, k)]
                 for who in (OTHER, WORKER) for k in ("Basic", "Standard")], [])
    L += start(["teleport %s %s dist 4" % (WORKER, OTHER), "protect %s off" % OTHER] + nokit + [
                "inv %s" % OTHER, "inv %s" % WORKER, "speed 1",
                "!healtime %s %s wound %d timeout 60 ~ finished=0 .*bandaging 0\\.0+ -> 0\\.0+" % (WORKER, OTHER, p["cut"]),
                "speed 0",
                "@echo BAL2,%s,%s,0,0,nokit,w0,heal_control,0,bandaging 0 -> 0,-" % (p["rows"].split("-")[0], p["prof"]),
                'give %s "Basic First Aid Kit" 5' % WORKER, "teleport %s %s dist 4" % (WORKER, OTHER),
                # Beaks carries a Standard First Aid Kit in Full-Base and bandages himself within 0.3 s
                # (flat 4.4/s at every Medic skill of the worker, m21-4080): take it off him first.
                ]) + wear(WORKER)
    # m22-4080 diagnosis of the flat medic rate (always ~77 bandaging in ~17.8 s, Medic 10 = Medic 90): which
    # input moves it at all? Medic 10 vs 90 with a double wound, and Medic 90 with a Standard kit only.
    # Separate prof name (<prof>_diag_*) so the fit never uses these points.
    none = EVENT_GEAR[0]
    # give 1 + drop the whole stack never fails, whatever the kits' remaining uses
    clear_kits = sum([['give %s "%s First Aid Kit" 1' % (WORKER, k), 'drop %s "%s First Aid Kit"' % (WORKER, k)]
                      for k in ("Basic", "Standard")], [])
    for label, skill, cut, kit in (("d1", 10, p["cut"] * 2, "Basic"), ("d2", 90, p["cut"] * 2, "Basic"),
                                   ("d3", 90, p["cut"], "Standard")):
        L += ["# --- %s diagnostic: skill %d, wound %d, %s kit only ---" % (label, skill, cut, kit),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill),
              ] + clear_kits + ['give %s "%s First Aid Kit" 1' % (WORKER, kit),
              "teleport %s %s dist 4" % (WORKER, OTHER)] + ready(AWAKE, r"inv %s ~ %s First Aid Kit" % (WORKER, kit))
        L += ["speed 1",
              "@set H healtime %s %s wound %d timeout 600 ~ (seconds_bandaging=[\\d.]+ bandaging [\\d.]+ -> [\\d.]+)"
              % (WORKER, OTHER, cut),
              "speed 0",
              echo(p["rows"], "%s_diag_%s_cut%d" % (p["prof"], kit.lower(), cut), skill, none, label, "heal", 0,
                   "${H}", "-"),
              "hunger %s 280" % WORKER]
    L += clear_kits + ['give %s "Basic First Aid Kit" 5' % WORKER]
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s %s dist 4" % (WORKER, OTHER)] + ready(AWAKE, r"inv %s ~ First Aid Kit" % WORKER)
        L += ["speed 1",
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
              "@set RS runspeed %s ~ (movement_speed=[\\d.]+ .*ideal_run_speed=[\\d.]+)"
              % WORKER,
              "@set T swimtime %s %d +x run ~ (swam [\\d.]+ m seconds=[\\d.]+)" % (WORKER, d),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "move", 0, "${T}", "${RS}")]
    return L + ["teleport %s %s dist 3" % (WORKER, OTHER)]


# Deep water for row 194: Full-Base has none within 3 km (m19-4080 findwater). Treefall's river, 14.6 km away:
# m23-4080: the old spot (-61979 97 33880, depth 3.3) reads water_level=very_shallow: Avarek wades, deep_seconds=0,
# every pg86 point invalid. findwater radius 5000 depth 25 -> depth 28.6 with a 300 m run along +z; there
# water_level=deep, swimming 90 swims 28.6 m in 1.1 s (too short: dist 60); low skill stops ~8.5 m short (harness swimtime stopped_short).
WATER_AT = "-62153 100 34366.6"
WATER_DIR = "+z"


def gen_swim(p):
    d = p["dist"]
    L = header(p, ["%s is teleported to deep water at Treefall (%s, findwater: depth 28.6, best run %s 300 m)"
                   % (WORKER, WATER_AT, WATER_DIR),
                   "per point and swims %d m along %s (swimtime). Result = swim_speed (deep distance / deep" % (d, WATER_DIR),
                   "seconds); runspeed adds the game's own swim_speed/max_swim_speed (harness 740ba0a+)."],
               ["water_level=deep at every start; deep_fraction near 1; own50 > none = swimming reads the hooked stat."])
    L += start(["teleport %s %s" % (WORKER, WATER_AT), "speed 1", "@sleep 8", "speed 0",
                "findwater %s radius 300 depth 25 ~ best_run=" % WORKER]) + wear(WORKER)
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s %s" % (WORKER, WATER_AT), "speed 1", "@sleep 3",
              "water %s ~ water_level=deep" % WORKER,
              "@set RS runspeed %s ~ (swim_speed=[\\d.]+ max_swim_speed=[\\d.]+)" % WORKER,
              "@set T swimtime %s %d %s ~ (deep_seconds=[\\d.]+ deep_dist=[\\d.]+)" % (WORKER, d, WATER_DIR),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "swim", 0, "${T}", "${RS}"),
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
                "where PGWatch"] + (["pg_statprobe on PGWatch"] if not sneaker_varies else [])) + wear(who)
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
    if p.get("owned_lock"):
        L += [
            "# An owner is required so these are real locked shackles, not an unowned test item.",
            '@set OWN spawn "Hungry Bandit" Drifters near %s dist 40 count 1 ~ spawned 1/1 [^:]+: .+? (#\\d+(?:/\\d+)?)' % WORKER,
            "setname ${OWN} PGLockOwner",
            "relation PGLockOwner 0",
            "shackle PGTarget owner PGLockOwner",
            "inv PGTarget",
        ]
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
    tail = ["stealth %s off" % WORKER, "teleport PGTarget %s dist 300" % WORKER, "kill PGTarget"]
    if p.get("owned_lock"):
        tail += ["kill PGLockOwner"]
        # Replace the generic mixed-chance header for this focused lock ownership control.
        L.insert(6, "# lock control: row 196 only, PGTarget wears shackles owned by PGLockOwner.")
    return L + tail


GEN = dict(craft=gen_craft, operate=gen_operate, research=gen_research, construct=gen_construct, heal=gen_heal,
           move=gen_move, swim=gen_swim, detect=gen_detect, chance=gen_chance)


def gen():
    os.makedirs(OUT_DIR, exist_ok=True)
    for p in P:
        L = GEN[p["kind"]](p)
        # PG 36D7474D+ pg_statprobe: after every point, which profession stats the game read through getStat
        # harness 44992a0+: a teleport can leave him where he was (m19-4080): repeat it until it lands
        L = ["@until 30 %s ~ moved=1" % x if x.startswith("teleport ") and "~" not in x else x for x in L]
        # m19-4080: a teleport of the worker after a load / clearjobs / long pause did nothing (moved=0, retries while
        # paused never land); after wake + 3 s of game time it always landed
        L = [y for x in L for y in (["wake %s" % WORKER, "speed 1", "@sleep 3", "speed 0", x]
                                     if x.startswith("@until 30 teleport %s " % WORKER) else [x])]
        # harness 90a0e33+: "speed N hold" resumes after the game's own pauses (m19-4080: a Bandit Demands event
        # paused the game at 50x and the scenario waited until its timeout)
        L = [x + " hold" if re.match(r"^speed (?!0$)[\d.]+$", x) else x for x in L]
        # (a read right after pg_bonus clears the reads pg_bonus itself made)
        L = [y for x in L for y in ([x, "pg_statprobe read"] if x.startswith(("@echo BAL2", "pg_bonus ")) else [x])]
        path = os.path.join(OUT_DIR, p["file"])
        with open(path, "w", newline="\n") as fh:
            fh.write("\n".join(L) + "\n")
        # FormulaScaling (PG C5CAC166+, off by default): the same sweep with pg_formulas on, professions "<prof>_fs"
        if p["kind"] in ("move", "swim", "chance") or p.get("prof") == "stealth":
            F = []
            for x in L:
                x = re.sub(r"^(@echo BAL2,[^,]*,)([^,]*),", r"\1\2_fs,", x)
                F.append(x)
                if x.startswith("pg_statprobe on "):
                    F.append("pg_formulas on ~ formulaScaling=1 hooks=8/8")
            F[0] = F[0] + "-fs"
            F.insert(2, "# variant: pg_formulas on (FormulaScaling), professions written as <prof>_fs")
            with open(path.replace(".txt", "-fs.txt"), "w", newline="\n") as fh:
                fh.write("\n".join(F + ["pg_formulas off"]) + "\n")
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
        # "Q/F": queue length / progress % of the first queued item (old files: queue only)
        def qf(x):
            parts = (x or "").split("/")
            return f(parts[0]), (f(parts[1]) if len(parts) > 1 else 0.0)
        (q0, f0), (q1, f1) = qf(a), qf(b)
        if q0 is None or q1 is None or f0 is None or f1 is None:
            return 0, window_s, 0, "missing queue"
        done = (q0 - q1) + (f1 - f0) / 100.0
        return done * 1000.0, window_s, int(done > 0 and q1 > 0), "queue empty" if q1 == 0 else ""
    if kind in ("operate", "research", "construct"):
        p0, p1 = f(a), f(b)
        if p0 is None or p1 is None:
            return 0, window_s, 0, "missing progress"
        done = p1 - p0
        # construction progress is NOT 0..1: it counts up to the site's material total (Small Shack: 7, then
        # complete=1; m21-4080). The P1 capture needs complete=0, so a finished site fails that line instead.
        note = ""
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


def short(x, n=90):
    return re.sub(r"\s+", " ", x).strip().replace(",", ";")[:n]


def prov_of(lines):
    """PROV line the runner writes on top of each .out (harness/PG DLL hashes, PG ini/rules, fixture, scenario hash);
    older outputs have none: 'prov=unknown' (usable only while nothing relevant changed since)."""
    for line in lines[:5]:
        m = re.match(r"^PROV (.*)$", line.strip())
        if m:
            return short(m.group(1), 300)
    return "prov=unknown"


def to_csv(paths):
    print("test_id,profession,base_skill,effective_bonus_pct,elapsed_game_seconds,result_count,valid,gear_loadout,notes,"
          "setup,evidence,provenance")
    flats = {}
    for path in paths:
        base = os.path.basename(path)
        lines = list(open(path, encoding="utf-8", errors="replace"))
        prov = prov_of(lines)
        fail = ""          # first failed command since the last point (its text is the invalid reason)
        stage = "setup"    # setup | ready | post
        control_bad = ""   # heal_control: the patient got bandaged with no kit on the medic
        for i, line in enumerate(lines):
            if re.search(r"@echo READY\b", line):
                stage = "ready"
            elif re.search(r"@echo POST\b", line):
                stage = "post"
            m = re.match(r"^FAIL\s+(\d+)\s+(.*)$", line)
            if m:
                # "fill ... nothing fitted" = the bench input was already full: a top-up, not a failure by itself.
                # The READY block's benches check reads the real bench state and decides.
                if "nothing fitted" not in line and not fail:
                    fail = "%s FAIL L%s %s" % (stage, m.group(1), short(m.group(2)))
                continue
            m = re.search(r"=> BAL2,([^,]*),([^,]*),(\d+),(\d+),([^,]*),([^,]*),([^,]*),([^,]*),([^,]*),(.*)$", line)
            if m:
                tid, prof, skill, bonus, gear, label, kind, win, a, b = m.groups()
                if "${" in a:
                    a = ""
                if "${" in b:
                    b = ""
                evidence = "L%d a=%s b=%s" % (i + 1, short(a.strip(), 60), short(b.strip(), 60))
                if kind == "heal_control":
                    hm = re.search(r"bandaging ([\d.]+) -> ([\d.]+)", a)
                    if fail or not hm:
                        control_bad = "heal control did not run (%s)" % (fail or "no healtime reply")
                    elif float(hm.group(2)) > float(hm.group(1)):
                        control_bad = "patient bandaged with no kit on the medic (%s): someone else treats him" % short(a, 60)
                    print("%s,%s,%s,%s,0,0,0,%s,%s,control,%s,%s" % (
                        tid, prof, skill, bonus, gear,
                        "%s heal_control %s %s" % (label, control_bad or "ok: no bandaging without a kit", base),
                        evidence, prov))
                    fail, stage = "", "setup"
                    continue
                count, secs, ok, note = measure(kind, f(win) or 0, a.strip(), b.strip())
                # A failed @set leaves its previous capture intact in kah.py: never fit that echoed stale value,
                # nor a point whose setup / readiness / post check failed (the reason names the failed check).
                setup = "ready=ok"
                if fail:
                    count, ok, note, setup = 0, 0, "invalid: " + fail, "ready=FAIL"
                if control_bad and kind == "heal":
                    count, ok, note = 0, 0, "invalid: " + control_bad
                fail, stage = "", "setup"
                if gear == "lab50":
                    prof, bonus = prof + "_lab50ctl", "0"
                # the next pg_statprobe read: which profession stats the game read through getStat in this point
                probe = ""
                for nxt in lines[i + 1:i + 3]:
                    pm = re.search(r"pg_statprobe read => statprobe \w+: (.*)$", nxt)
                    if pm:
                        probe = " probe[" + pm.group(1).strip().replace(",", "/") + "]"
                        break
                extra = (" b=" + b.strip().replace(",", ";")) if b.strip() not in ("", "-") and kind != "craft" else ""
                print("%s,%s,%s,%s,%s,%s,%d,%s,%s,%s,%s,%s" % (
                    tid, prof, skill, bonus, secs, count, ok, gear,
                    "%s %s %s %s%s%s" % (label, kind, note, base, extra, probe), setup, evidence, prov))
                if ok and gear == "none" and float(secs or 0) > 0:
                    flats.setdefault((base, prof), {})[int(skill)] = count / float(secs)
                continue
            # pg-14 labouring curve (rows 177-183, 30 game minutes per window): "skill 50 +25%: ... output_progress=x"
            m = re.search(r"=> skill (\d+) (no gear|\+(\d+)%|set 25\+25): .*output_progress=([-\d.]+)", line)
            if m:
                skill, what, pct, prog = m.groups()
                bonus = 50 if what.startswith("set") else int(pct or 0)
                gear = "none" if what == "no gear" else ("set25+25" if what.startswith("set") else "own%d" % bonus)
                print("177,labouring,%s,%d,1800,%s,1,%s,pg-14 %s,ready=unchecked,L%d,%s" % (
                    skill, bonus, float(prog) * 1000.0, gear, base, i + 1, prov))
                continue
            m = re.search(r"=> BAL,([^,]+),([^,]+),(\d+),(\d+),(\d+),(\d*),(\d*),([^,]+),(\S+)", line)
            if m:  # the old pg-52/53 format
                tid, prof, skill, bonus, secs, q0, q1, gear, label = m.groups()
                count, _, ok, note = measure("craft", float(secs), q0, q1)
                if gear == "lab50":
                    prof, bonus = prof + "_lab50ctl", "0"
                print("%s,%s,%s,%s,%s,%s,%d,%s,%s,ready=unchecked,L%d,%s" % (
                    tid, prof, skill, bonus, secs, count, ok, gear, "%s craft %s %s" % (label, note, base), i + 1, prov))
    # 0 script failures is not valid balance data: a curve whose skill-10 and skill-90 throughputs (no gear) are
    # within 3% is flagged for investigation before any fit (m21/m22: medic flat ~4.37/s at every skill).
    for (base, prof), pts in sorted(flats.items()):
        if 10 in pts and 90 in pts and pts[10] > 0 and abs(pts[90] / pts[10] - 1.0) < 0.03:
            sys.stderr.write("FLAT %s %s: skill 10 %.4g vs skill 90 %.4g per s: investigate before fitting\n"
                             % (base, prof, pts[10], pts[90]))


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
