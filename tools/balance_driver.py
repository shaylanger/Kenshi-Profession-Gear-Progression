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
gate: the validation gate files (tests/ingame/full-base/gate/pg-gate-*.txt): GATE_POINTS per profession, same
     fixture and method as the matrix; run them before any full matrix (RUN_ORDER.md).
gatecheck: one GATE line per profession from csv files of gate outputs (valid points, drift bracket, equal game
     conditions, skill 25 vs 90 response, gear gain vs the gain the game's own hooked values predict).
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
  python3 tools/balance_driver.py gate                (writes the gate files)
  python3 tools/balance_driver.py gatecheck gate.csv...
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "tests", "ingame", "full-base")

# Fixture Testing-Save-Full-Base: squad Beaks + Avarek. m29-4080: the worker is Beaks (stays the selection). The
# wounds factor 0.25 first blamed on a "crippled Avarek" was the harness protect/health bug (stun damage written to max,
# fixed in harness 6aa5685); it quartered every skill read in batches 17-20 and explains the athletics cap.
WORKER = "Beaks"
OTHER = "Avarek"
GEAR_ITEM = "Iron Hat"       # given + worn by whoever's stat is varied; its affix is forced per point
NEEDS = ("ProfessionGearProgression.dll 36D7474D+ (pg_statprobe), harness c5a5c88+ (give artifacts, research start any; 90a0e33: teleport moved=/bed, speed hold; 6933d0f: fill topup, drop all; 616121b: benches near; c4fa520: pin, d91fe88: face) (KAH 24: chance, detect, "
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
# PG validation gate (Shay 2026-10-04): before any full matrix, per profession on the same fixture and method:
# low skill/no gear, low skill/+50% own gear, high skill/no gear, then low skill/no gear again (drift bracket).
# Low = 25, not 10: gear multiplies the skill (expected = vanilla x 1.5, pg_bonus), so at 10 the +50% is 5 skill
# points, inside the window-to-window noise; at 25 it is 12.5. `python3 tools/balance_driver.py gate` writes
# tests/ingame/full-base/gate/pg-gate-*.txt; `gatecheck <csv>` prints one GATE line per profession.
GATE_LOW, GATE_HIGH = 25, 90
GATE_POINTS = [("g1", GATE_LOW, ("none", None, 0)), ("g2", GATE_LOW, ("own50", "OWN", 50)),
               ("g3", GATE_HIGH, ("none", None, 0)), ("g4", GATE_LOW, ("none", None, 0))]
GATE = False
CHANCE_GEAR = [("own2", "OWN", 2), ("own5", "OWN", 5), ("own10", "OWN", 10), ("own25", "OWN", 25),
               ("own50", "OWN", 50), ("own100", "OWN", 100), ("lab50", "labouring", 50)]

# m25-4080 probe: craft progress per GAME hour falls with game speed (Gears, Robotics Bench, robotics 50: speed 3 =
# 10.2/10.9 %/h, speed 20 = 3.8/5.8 %/h; the frame step is likely clamped). Craft and research windows run at speed 3;
# 30-game-minute windows (~1.5 real min each) give ~5 % first-item progress for slow items like Gears.
CRAFT_SPEED = 3
CRAFT_WINDOW = 30

# setstat names (harness) and pg stat names (pg_force_affix / pg_bonus).
P = [
    # ---- crafting benches (window = queue drop) ----
    dict(file="pg-52-balance-weapon-smithing.txt", rows="188", kind="craft", prof="weapon_smithing",
         setstat="weapon_smith", bench="Weapon Smith", item="Sickle", give=[("Steel Bars", 10), ("Fabrics", 10)],
         window=CRAFT_WINDOW, prep=['research "Basic Weapon Smithing"', 'research "Basic Weapon Grades"',
                          'research "Utility Weapons"', 'research "Katanas"', 'research "Basic Weapon Grades 3"'],
         check=r'benches 60 crafts ~ Weapon Smith[^|]*Sickle'),
    dict(file="pg-53-balance-armour-smithing.txt", rows="189", kind="craft", prof="armour_smithing",
         setstat="armour_smith", bench="Clothing", item="Rag Shirt", give=[("Fabrics", 10)], window=CRAFT_WINDOW,
         prep=['blueprint "Rag Shirt"'], check=r'benches 60 crafts ~ Clothing[^|]*Rag Shirt'),
    dict(file="pg-80-balance-crossbow-smithing.txt", rows="190", kind="craft", prof="crossbow_smithing",
         setstat="crossbow_smith", bench="Crossbow Crafting", item="Junkbow",
         give=[("Steel Bars", 10), ("Hinge", 10)], window=CRAFT_WINDOW,
         prep=['research "Crossbow Crafting"'], check=r'benches 60 crafts ~ Crossbow Crafting[^|]*Junkbow'),
    dict(file="pg-81-balance-robotics.txt", rows="186", kind="craft", prof="robotics", setstat="robotics",
         bench="Robotics Bench", item="Gears", give=[("Iron Plates", 15)], window=CRAFT_WINDOW,
         prep=['research "Robotics"'], check=r'benches 60 crafts ~ Robotics Bench[^|]*Gears'),
    dict(file="pg-82-balance-cooking.txt", rows="187", kind="craft", prof="cooking", setstat="cooking",
         bench="Cooking Stove", item="Dried Meat", give=[("Raw Meat", 15)], window=CRAFT_WINDOW,
         # Full-Base has no Cooking Stove (the Bread Oven runs without an operator): build one (m19-4080)
         prep=['teleport %s %s dist 30' % (WORKER, OTHER), 'build "Cooking Stove" near %s dist 10' % WORKER],
         check=r'benches 60 crafts ~ Cooking Stove[^|]*Dried Meat'),
    # ---- work over time ----
    dict(file="pg-90-balance-farming.txt", rows="161-176", kind="operate", prof="farming", setstat="farming",
         building="Wheat Farm L", fill=("Water", 40), window=30, extra_skills=[25, 75],
         # m31-5090: a window that ran at night read light 1.0 -> 0 and its growth differed: every point first waits
         # (speed 20) for daylight, READY and POST assert it, so no point is measured in the dark.
         daylight=True),
    dict(file="pg-54-research.txt", rows="184", kind="research", prof="science", setstat="science",
         building="Research Bench", window=30,
         ),
    dict(file="pg-83-balance-engineering.txt", rows="185", kind="construct", prof="engineering",
         setstat="engineering", building="Small Shack", window=5),
    # ---- timed events ----
    dict(file="pg-84-balance-medic.txt", rows="191", kind="heal", prof="medic", setstat="medic", cut=30),
    dict(file="pg-85-balance-athletics.txt", rows="193", kind="move", prof="athletics", setstat="athletics", dist=300),
    # m29-5090 gate: 40 units (~4 m: harness distances are game units) ran in 0.8-1.0 s at every Athletics while
    # stats_run_speed went 82.5 -> 115: acceleration-bound, flat. 300 units ~ 2.6-3.6 s at run speed resolves it.
    dict(file="pg-86-balance-swimming.txt", rows="194", kind="swim", prof="swimming", setstat="swimming", dist=60),
    dict(file="pg-87-balance-stealth.txt", rows="195", kind="detect", prof="stealth", setstat="stealth",
         side="sneaker", dist=100, timeout=180),
    dict(file="pg-89-balance-perception.txt", rows="199", kind="detect", prof="perception", setstat="perception",
         side="observer", dist=100, timeout=180, sneaker_stealth=70),
    # m31-5090: gate 89 at sneaker stealth 30 was flat (seen 3.0 s, maybe 0.1 s at every Perception): 70 makes the
    # maybe->seen phase long enough for Perception to move it
    # ---- the game's own chances ----
    # m31-5090: KenshiLib's Character::getLockpickChance is a dead stub in 1.0.65 (returns 0.0 at every skill: the
    # m29 0.0000 readings). Harness DE9D9A54+ `chance lockpick` calls the game's DoorLock chance fn instead
    # (method=game); the gate asserts it so an old harness can never pass a 0 again.
    # A player-owned/unoccupied cage can report zero at every skill. Compare a real
    # locked shackle with an explicit foreign owner before treating row 196 as no response.
    dict(file="pg-92-balance-owned-lock.txt", rows="196", kind="chance", owned_lock=True, subs=[
        dict(row="196", prof="lockpicking", setstat="lockpicking",
             what="lockpick PGTarget", key="lockpick_chance",
             check=" .*method=game"),
    ]),
    dict(file="pg-88-balance-chances.txt", rows="196-198", kind="chance", subs=[
        dict(row="196", prof="lockpicking", setstat="lockpicking", what='lockpick "Prisoner Cage"',
             key="lockpick_chance",
             check=" .*method=game"),
        dict(row="197", prof="assassination", setstat="assassination", what="ko PGTarget", key="stealth_ko_chance"),
        dict(row="198", prof="thievery", setstat="thievery", what='steal PGTarget item "Iron Plates"',
             key="steal_chance"),
    ]),
]

NUM = r"([-\d.]+)"
# m23b-4080: @wait-game overshoots (asked 5/20 game min, got 8.5/21.6-33): measure every window with the game clock
# and echo "T<h0>_<h1>" as the window; csv turns it into game seconds.
TS0 = r"@set T0 time ~ game_hours=([\d.]+)"
TS1 = r"@set T1 time ~ game_hours=([\d.]+)"
WIN = "T${T0}_${T1}"


def pgstat(prof, stat):
    return prof if stat == "OWN" else stat


def gear_lines(who, prof, gear):
    label, stat, pct = gear
    lines = ["pg_clear %s ~ cleared affixes" % who]
    if stat:
        s = pgstat(prof, stat)
        lines.append('pg_force_affix %s "%s" %s %d tier 6 ~ affixes=' % (who, GEAR_ITEM, s, pct))
        lines.append("pg_bonus %s %s ~ equipped_bonus=%d%% .*match=1" % (who, s, pct))
    elif GATE:  # gate: the no-gear points also log the game's own base / vanilla / hooked values (native evidence)
        lines.append("pg_bonus %s %s ~ equipped_bonus=0%% .*match=1" % (who, prof))
    return lines


def cond(who, stat):
    """The game's condition multiplier on the measured stat (harness `stat`, CCA2984F+: mod= hunger= wounds= dark=
    robot= weather= gear= light=). csv puts it in the notes; gatecheck rejects a gate whose points ran under
    different conditions (m28: base 50 read 27.5 early and 12.5 later in the same file)."""
    return "stat %s %s" % (who, stat)


def echo(row, prof, skill, gear, label, kind, window_s, a, b):
    bonus = gear[2] if gear[1] == "OWN" else 0
    return "@echo BAL2,%s,%s,%d,%d,%s,%s,%s,%s,%s,%s" % (row.split("-")[0], prof, skill, bonus, gear[0], label, kind,
                                                       window_s, a, b)


def points(skills, gears, repeats, base=BASE):
    """[(label, skill, gear)]: skills without gear, then repeats x gears at the base skill."""
    if GATE:
        return list(GATE_POINTS)
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
            # m29-4080: the fixture's Avarek is wounded (in bed): stat showed wounds=0.25 -> mod=0.25 on every skill,
            # so every point measured a quarter of the set skill. Full health + blood for both, checked before
            # every measurement (cond_ready: wounds=1).
            "health %s 100" % WORKER, "health %s 100" % OTHER, "blood %s 100%%" % WORKER, "blood %s 100%%" % OTHER,
            "hunger %s 280" % WORKER, "hunger %s 280" % OTHER,
            # select the worker: an unselected Avarek walks into the base's Bed and lies there (m19-4080)
            "select %s" % WORKER, "clearjobs %s" % WORKER, "pg_statprobe on %s" % WORKER,
            # m29-4080: the harness searches buildings/benches/fill around the FIRST squad member (Avarek = OTHER),
            # and Avarek walks back to his bed (320 m) within seconds: pin him (harness pin) where he is;
            # anchor() moves the pin next to the worker after every worker teleport to a building.
            "wake %s" % OTHER, "pin %s ~ pinned" % OTHER] + list(extra)


def anchor():
    return ["pin %s off ~ unpinned" % OTHER, "@until 30 teleport %s %s dist 8 ~ moved=1" % (OTHER, WORKER),
            "pin %s ~ pinned" % OTHER]


def with_anchor(L):
    """After every teleport of the worker to a building: bring the pinned first squad member (search origin) along."""
    out = []
    for x in L:
        out.append(x)
        if re.match(r"^(@until \d+ )?teleport %s building " % WORKER, x):
            out += anchor()
    return out


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
DAYLIGHT = r"stat %s %s ~ light=(0\.[5-9]|1)"  # % (npc, stat): Character::getLightLevel >= 0.5 (day)
# m34-4080 gate 90: a window that started at 21:18 (light 0.72) ended at dusk (light 0.06, POST failed): a daylight
# point also starts only between 07:00 and 18:59 game time, so the whole window runs before dusk (~21:30)
DAYTIME = r"time ~ time=(0[7-9]|1[0-8]):"


def powered(b):
    # m22-4080: crafting/research stopped after the first windows with the worker at the bench (job kept):
    # power is held constant (harness `power ... supply`, c5a5c88+ also answers ok for benches that use none)
    # and checked before every window
    return r'building "%s" 60 ~ power_on=1 .*broken=0 .*(wants=0(\.0+)?\s|supplied=1)' % b


def bench_has(bench, item):
    # the bench's own entry only: " || <bench> ... | in1 6x3 limit=1: [Steel Bars x7 @..]"; '||' separates benches.
    # m23-4080: an INPUT section (in1, in2, ...), never "out": materials in out don't feed the craft (stalled at 2.1%)
    # near WORKER (harness 616121b): benches searched around the worker, not the squad leader who may wander
    return r"benches 60 near %s ~ %s[^|]*(?:\|[^|]+)*?\| in\d[^|]*\[%s x[1-9]" % (WORKER, bench, item)


def timed_points(p):
    skills = sorted(set(TIMED_SKILLS + p.get("extra_skills", [])))
    return points(skills, TIMED_GEAR, TIMED_REPEATS)


# ---------------------------------------------------------------- kinds
def gen_craft(p):
    w, bench = p["window"], p["bench"]
    sp = p.get("speed", CRAFT_SPEED)
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
    q = '@set Q%d benches 60 near ' + WORKER + ' ~ ' + bench + r'[^|]*queue=(\d+)'
    fpat = '@set F%d benches 60 near ' + WORKER + ' ~ ' + bench + r'[^|]*queue=\d+ \(first: [^|]*? ([\d.]+)%%\)'
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
        # topup (harness 6933d0f): a refill of an already full input answers ok; 0 held still fails, and the
        # READY bench check reads the real bench state (m23b-4080: full-input refills made every craft row FAIL)
        L += ['fill "%s" "%s" %d radius 60 topup' % (bench, it, n) for it, n in p["give"]]
        L += ready(AWAKE, *[bench_has(bench, it) for it, _ in p["give"]] +
                   [r"benches 8 near %s ~ %s[^|]*queue=[1-9]" % (WORKER, bench), HASJOB, powered(bench)])
        L += [q % 0, fpat % 0,
              TS0, "speed %d" % sp, "@wait-game %d 900" % w, "speed 0", TS1,
              "where %s" % WORKER, "jobs %s" % WORKER]
        # m22-4080 pg-52 w10: he walked 500 m away during the window (benches 60 then found another bench)
        L += post(AWAKE, r"benches 8 near %s ~ %s" % (WORKER, bench))
        L += [q % 1, fpat % 1,
              echo(p["rows"], p["prof"], skill, gear, label, "craft", WIN, "${Q0}/${F0}", "${Q1}/${F1}"),
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
        day = DAYLIGHT % (WORKER, p["setstat"])
        if p.get("daylight"):
            L += ["speed 20", "@until 2400 " + DAYTIME, "@until 1200 " + day, "speed 0"]
        L += ['fill "%s" %s %d radius 1000 topup' % (b, p["fill"][0], p["fill"][1])]
        L += ready(AWAKE, HASJOB, *([day, DAYTIME] if p.get("daylight") else []))
        L += ['pg_operate "%s" reset radius 100 near %s ~ \\(reset\\)' % (b, WORKER),
              TS0, "speed 10", "@wait-game %d 900" % w, "speed 0", TS1]
        if p.get("daylight"):
            L += post(day)
        L += ['@set OP pg_operate "%s" radius 100 near %s ~ output_progress=%s' % (b, WORKER, NUM),
              echo(p["rows"], p["prof"], skill, gear, label, "operate", WIN, "0", "${OP}"),
              "hunger %s 280" % WORKER]
    return L + ["clearjobs %s" % WORKER]


def gen_research(p):
    w = p["window"]
    L = header(p, ["%s researches at the %s (job); research status raw progress points of the first queued tech per"
                   % (WORKER, p["building"]),
                   "%d-game-minute window at speed %d. research start any 3 queues up to 3 techs the game would" % (w, CRAFT_SPEED),
                   "start now (longest first); the first is measured; progress resets to the next tech",
                   "when one completes (then that window's BAL2 is invalid: P1 < P0)."],
               ["progress rises in every window (else no power/bench level: research status desk_level/benches);",
                "own50 > none = research reads the hooked Science (ResearchBuilding::operate is not scaled by PG)."])
    # Full-Base: the bench is ~520 m from the squad and research costs Books (can_pay=0 without, m21-4080)
    # m23-4080: the research cost item is ITEM "Book" (`find item book` -> [Book]); "Books" = no data named. Hand-picked tech names were
    # finished / missing / unpayable in this save, so the harness picks startable techs (longest first)
    L += start(['@until 30 teleport %s building "%s" dist 4 radius 1500 ~ moved=1' % (WORKER, p["building"]),
                # m25-4080 probe: Books in the worker's inventory do not pay (canPayCosts skipped cost=27 with 40
                # carried); Books in the bench inventory do (fill -> started=3, cost skips 27 -> 10)
                'power "%s" supply radius 60' % p["building"],
                'fill "%s" "Book" 40 radius 60 topup' % p["building"], "research status",
                "research start any 3 ~ started=[1-9]",
                "research status ~ queue=[1-9]",
                'job %s "%s" radius 60' % (WORKER, p["building"])]) + wear(WORKER)
    L += ["speed 10", "@wait-game 5 600", "speed 0"]
    for label, skill, gear in timed_points(p):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ['@until 30 teleport %s building "%s" dist 4 radius 1500 ~ moved=1' % (WORKER, p["building"])]
        L += ready(AWAKE, HASJOB, r"research status ~ queue=[1-9]", r"research status ~ ^(?!.*power_off)", powered(p["building"]))
        # m25-4080 pg-54: progress= has 1 decimal (0.1 steps); raw= is the game's own progress points (0 -> ~430 for an
        # 8-hour tech), and researchers=/rate= read 0 while paused, so the post check is the job, not researchers
        L += ["@set P0 research status ~ raw=%s" % NUM,
              TS0, "speed %d" % CRAFT_SPEED, "@wait-game %d 900" % w, "speed 0", TS1]
        L += post(AWAKE, HASJOB)
        L += ["@set P1 research status ~ raw=%s" % NUM,
              echo(p["rows"], p["prof"], skill, gear, label, "research", WIN, "${P0}", "${P1}"),
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
              TS0, "speed 10", "@wait-game %d 900" % w, "speed 0", TS1,
              '@set P1 construction "%s" ~ progress=%s complete=0' % (b, NUM),
              echo(p["rows"], p["prof"], skill, gear, label, "construct", WIN, "${P0}", "${P1}"),
              "clearjobs %s" % WORKER,
              'unbuild "%s"' % b,
              "hunger %s 280" % WORKER]
    return L


# m31-5090: the Basic First Aid Kit caps the Medic skill (Medic 10 and 90 both ~35 s per wound), the Standard kit
# scales (diag d3/d4: ~12.4 s at Medic 10, ~4.5 s at Medic 90): the measured points use the Standard kit.
HEAL_KIT = "Standard"


def gen_heal(p):
    L = header(p, ["%s (medic, 5 Standard First Aid Kits) treats %s: per point healtime ... wound %d gives %s full"
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
    # m23b-4080: kits with uses don't stack, so give 1 + drop left one kit (Avarek bandaged Beaks in the control):
    # harness b444cf7 `drop ... all` drops every instance (ok with 0); inv must then show no kit at all.
    nokit = ['drop %s "%s First Aid Kit" all' % (who, k) for who in (OTHER, WORKER) for k in ("Basic", "Standard")]
    L += start(["teleport %s %s dist 4" % (WORKER, OTHER), "protect %s off" % OTHER] + nokit + [
                "inv %s ~ ^(?!.*First Aid Kit)" % OTHER, "inv %s ~ ^(?!.*First Aid Kit)" % WORKER, "speed 1",
                "!healtime %s %s wound %d timeout 60 ~ finished=0 .*bandaging 0\\.0+ -> 0\\.0+" % (WORKER, OTHER, p["cut"]),
                "speed 0",
                "@echo BAL2,%s,%s,0,0,nokit,w0,heal_control,0,bandaging 0 -> 0,-" % (p["rows"].split("-")[0], p["prof"]),
                'give %s "%s First Aid Kit" 5' % (WORKER, HEAL_KIT), "teleport %s %s dist 4" % (WORKER, OTHER),
                # Beaks carries a Standard First Aid Kit in Full-Base and bandages himself within 0.3 s
                # (flat 4.4/s at every Medic skill of the worker, m21-4080): take it off him first.
                ]) + wear(WORKER)
    # m22-4080 diagnosis of the flat medic rate (always ~77 bandaging in ~17.8 s, Medic 10 = Medic 90): which
    # input moves it at all? Medic 10 vs 90 with a double wound, and Medic 90 / 10 with a Standard kit only.
    # Separate prof name (<prof>_diag_*) so the fit never uses these points.
    none = EVENT_GEAR[0]
    # give 1 + drop the whole stack never fails, whatever the kits' remaining uses
    clear_kits = ['drop %s "%s First Aid Kit" all' % (WORKER, k) for k in ("Basic", "Standard")]
    for label, skill, cut, kit in (("d1", 10, p["cut"] * 2, "Basic"), ("d2", 90, p["cut"] * 2, "Basic"),
                                   ("d3", 90, p["cut"], "Standard"), ("d4", 10, p["cut"], "Standard")):
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
    L += clear_kits + ['give %s "%s First Aid Kit" 5' % (WORKER, HEAL_KIT)]
    for label, skill, gear in points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS):
        L += ["# --- %s: skill %d, gear %s ---" % (label, skill, gear[0]),
              "setstat %s %s %d" % (WORKER, p["setstat"], skill)] + gear_lines(WORKER, p["prof"], gear)
        L += ["teleport %s %s dist 4" % (WORKER, OTHER)] + ready(AWAKE, r"inv %s ~ %s First Aid Kit" % (WORKER, HEAL_KIT))
        L += ["speed 1",
              "@set H healtime %s %s wound %d timeout 600 ~ (seconds_bandaging=[\\d.]+ bandaging [\\d.]+ -> [\\d.]+)"
              % (WORKER, OTHER, p["cut"]),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "heal", 0, "${H}", "-"),
              'give %s "%s First Aid Kit" 1' % (WORKER, HEAL_KIT),
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


# m31-5090 (gate 87/89): a random spawn spot decided the result (los 0.2 vs 0.7, a terrain-blocked spot never
# saw at all). With DETECT_SPOT set, every point teleports the sneaker to that fixed open spot and pins the observer
# DETECT dist from him (same direction every point); the READY check also requires line of sight >= 0.5
# (harness A447F08E+: `face` reports los=). Choose the spot with tests/ingame/full-base/pg-detect-spot-probe.txt.
DETECT_SPOT = (-75966.1, 327.2, 33085.7)   # m33 5090 spot probe P5 (600 m from base, los=1, neighbours P6/P7 los=1); (x, y, z) or None = sneaker brought next to a randomly spawned observer (old method)
LOS_MIN = r"los=(0\.[5-9]\d*|1\.0+)"

def upto(m):
    """Regex for the integers 1..m (no leading zero), m >= 1."""
    if m < 10:
        return "[1-%d]" % m
    a, b = divmod(m, 10)
    alts = ["[1-9]"] + ([r"[1-%d]\d" % (a - 1)] if a > 1 else []) + ["%d[0-%d]" % (a, b)]
    return "|".join(alts)


def gen_detect(p):
    d, to = p["dist"], p["timeout"]
    sneaker_varies = p["side"] == "sneaker"
    # m23b-4080 (pg-87-fs, pg-89): one observer for the whole file did not hold: 3 s after "teleport PGWatch ... dist 20"
    # detecttime measured him 214 m away, ~90 s later he was gone (spawned squad cleaned up). Now a fresh observer is
    # spawned at distance d for every point (own name PGW<n>, killed + moved away after the point) and a READY check
    # asserts he is still within d+10 m right before detecttime; otherwise the point is invalid (setup).
    L = header(p, ["a fresh neutral observer per point (Hungry Bandit, Drifters, renamed PGW<n>) spawned %d m from %s,"
                   % (d, WORKER),
                   "who sneaks (detecttime: sneak on). Result = game seconds until seen",
                   "(timeout %d s). The %s's skill/gear varies (%s)." % (to, p["side"], p["prof"]),
                   "The observer is killed and moved away after every point."],
               ["seen=1 in most points (seen=0 everywhere: this observer never notices a sneaker, send the .out to",
                "the PG agent); stealth: seconds_to_seen grows with Stealth; perception: it shrinks with Perception."])
    L += start(["teleport %s %s dist 60" % (WORKER, OTHER), "setstat %s stealth %d" % (WORKER, p.get("sneaker_stealth", 30))]) +         (wear(WORKER) if sneaker_varies else [])
    # m29-5090 gate: dist 20 is 20 game units (~2 m): seen in 1.5-3.4 s at every skill (flat); 100 units ~ 10 m.
    # The READY check was "where <obs>", whose dist= is from the search origin (pinned Avarek, ~80 away), not from
    # the sneaker: every point read invalid. "face <obs> <worker>" reports the observer-to-sneaker distance.
    lim = d + 10
    # m34-4080: the old pattern built a broken character class from 200 up ("[1-19]"); any distance now works
    near = r"dist=(\d|(%s)\d)\.\d observer_ko=0" % upto(lim // 10 - 1)
    for n, (label, skill, gear) in enumerate(points(EVENT_SKILLS, EVENT_GEAR, EVENT_REPEATS), 1):
        obs = "PGW%d" % n
        who = WORKER if sneaker_varies else obs
        L += ["# --- %s: %s %d, gear %s ---" % (label, p["prof"], skill, gear[0]),
              "stealth %s off" % WORKER,
              ("@until 30 teleport %s %s %s %s ~ moved=1" % ((WORKER,) + tuple(DETECT_SPOT)) if DETECT_SPOT
               else "teleport %s %s dist 60" % (WORKER, OTHER)),
              "@set OBS spawn \"Hungry Bandit\" Drifters near %s dist %d count 1 ~ spawned 1/1 [^:]+: .+? (#\d+(?:/\d+)?)"
              % (WORKER, d),
              "setname ${OBS} %s" % obs,
              # m27-4080 (pg-87/87-fs/89 batch19): the observer still ran 200-3000 m away during detecttime even
              # with the sneaker teleported to him (HOLD_POSITION did not help): pin him (harness c4fa520) where he
              # spawned (game paused), facing the sneaker, until the point is measured
              ("pin %s at %s dist %d face %s ~ pinned" % (obs, WORKER, d, WORKER) if DETECT_SPOT
               else "pin %s face %s ~ pinned" % (obs, WORKER)),
              "relation %s 0" % obs, "setstat %s perception 30" % obs]
        if not sneaker_varies:
            L += ["pg_statprobe on %s" % obs] + wear(obs)
        L += ["setstat %s %s %d" % (who, p["setstat"], skill)] + gear_lines(who, p["prof"], gear)
        # m25-4080 batch19 pg-87: 2 of the first 3 observers stood ~200 m away 3 s after the spawn (the game moved
        # them out of the base layout), w2 was 16 m away: bring the sneaker to wherever the observer stands
        if not DETECT_SPOT:
            L += ['@until 30 teleport %s %s dist %d ~ moved=1' % (WORKER, obs, d)]
        L += ["face %s %s" % (obs, WORKER)]
        L += ["speed 1", "@sleep 3", "speed 0"] + ready("face %s %s ~ %s %s" % (obs, WORKER, near, LOS_MIN)) +              ["speed 1",
              "@set T detecttime %s %s timeout %d ~ (seen=\d seconds_to_seen=\S+ seconds_to_maybe=\S+)" % (WORKER, obs, to),
              "speed 0",
              echo(p["rows"], p["prof"], skill, gear, label, "detect_" + p["side"], to, "${T}", "-"),
              "stealth %s off" % WORKER, "pin %s off ~ unpinned" % obs, "kill %s" % obs,
              "teleport %s %s dist 300" % (obs, WORKER)]
    return L + ["stealth %s off" % WORKER, "teleport %s %s dist 3" % (WORKER, OTHER)]

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
        if GATE:
            pts = list(GATE_POINTS)
        for label, skill, gear in pts:
            L += ["# --- %s %s: skill %d, gear %s ---" % (sub["row"], prof, skill, gear[0]),
                  "setstat %s %s %d" % (WORKER, sub["setstat"], skill)] + gear_lines(WORKER, prof, gear)
            L += ["teleport PGTarget %s dist 3" % WORKER,
                  "@set C chance %s %s ~ %s=%s%s" % (WORKER, sub["what"], sub["key"], NUM, sub.get("check", "")),
                  echo(sub["row"], prof, skill, gear, label, "chance", 0, "${C}", "-")]
    tail = ["stealth %s off" % WORKER, "teleport PGTarget %s dist 300" % WORKER, "kill PGTarget"]
    if p.get("owned_lock"):
        tail += ["kill PGLockOwner"]
        # Replace the generic mixed-chance header for this focused lock ownership control.
        L.insert(6, "# lock control: row 196 only, PGTarget wears shackles owned by PGLockOwner.")
    return L + tail


GEN = dict(craft=gen_craft, operate=gen_operate, research=gen_research, construct=gen_construct, heal=gen_heal,
           move=gen_move, swim=gen_swim, detect=gen_detect, chance=gen_chance)


MEASURE_START = ("@set H healtime", "@set T swimtime", "@set T detecttime", "@set C chance")


def with_conditions(L):
    """Before every measurement (window start, healtime, swimtime, detecttime, chance) and after every window end:
    the game's condition multiplier on the stat the point varies (the last setstat), as native evidence."""
    out, last = [], None
    for x in L:
        m = re.match(r"^setstat (\S+) (\S+) \d+$", x)
        if m:
            last = cond(m.group(1), m.group(2))
        if last and (x == TS0 or x.startswith(MEASURE_START)):
            out.append(last + r" ~ wounds=1\.0000")  # m29: a wounded measurer quarters the skill (setup failure)
            if x.startswith("@set T swimtime"):
                out.append("runspeed %s" % WORKER)
        out.append(x)
        if last and x == TS1:
            out.append(last)
    return out


def gen():
    out_dir = os.path.join(OUT_DIR, "gate") if GATE else OUT_DIR
    os.makedirs(out_dir, exist_ok=True)
    for p in P:
        L = with_anchor(with_conditions(GEN[p["kind"]](p)))
        if GATE:
            L[0] = "# id: PG-gate-%s" % os.path.splitext(p["file"])[0][3:]
            L.insert(2, "# gate: validation gate (RUN_ORDER.md): %s, same fixture and method as the matrix file"
                     % ", ".join("%s skill %d gear %s" % (lb, sk, g[0]) for lb, sk, g in GATE_POINTS))
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
        # FormulaScaling is on by default since PG 1db9fe7 (Shay 2026-10-04): the vanilla sweep sets it off
        # explicitly so its numbers stay vanilla on any build; the -fs variant below turns it on instead
        assert any(x.startswith("pg_statprobe on ") for x in L), p["file"]
        L = [y for x in L for y in ([x, "pg_formulas off ~ formulaScaling=0"]
                                     if x.startswith("pg_statprobe on ") else [x])]
        path = os.path.join(out_dir, p["file"].replace("pg-", "pg-gate-", 1) if GATE else p["file"])
        with open(path, "w", newline="\n") as fh:
            fh.write("\n".join(L) + "\n")
        # FormulaScaling (PG C5CAC166+; default on since 1db9fe7): the same sweep with pg_formulas on, professions "<prof>_fs"
        if p["kind"] in ("move", "swim", "chance") or p.get("prof") in ("stealth", "medic", "engineering"):
            F = []
            for x in L:
                x = re.sub(r"^(@echo BAL2,[^,]*,)([^,]*),", r"\1\2_fs,", x)
                if x == "pg_formulas off ~ formulaScaling=0":
                    x = "pg_formulas on ~ formulaScaling=1 hooks=8/8"
                F.append(x)
            F[0] = F[0] + "-fs"
            F.insert(2, "# variant: pg_formulas on (FormulaScaling), professions written as <prof>_fs")
            with open(path.replace(".txt", "-fs.txt"), "w", newline="\n") as fh:
                fh.write("\n".join(F + ["pg_formulas off"]) + "\n")
        n = sum(1 for x in L if x.startswith("@echo BAL2"))
        print("wrote %-42s rows %-8s kind %-9s points %d" % (p["file"], p["rows"], p["kind"], n))


# ---------------------------------------------------------------- csv
def f(x):
    m = re.match(r"^T([\d.]+)_([\d.]+)$", str(x).strip())
    if m:  # game-clock window (m23b): game hours -> game seconds
        return (float(m.group(2)) - float(m.group(1))) * 3600.0
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
        # m29b-5090: the time until the observer first glances ("maybe") is random (1.7-63 s at the same skill);
        # the maybe->seen phase is what Stealth/Perception set (s25 1.7-2.2 s, s90 10.2 s). Use that phase when the
        # reply has seconds_to_maybe (generator m29b+).
        mm = re.search(r"seconds_to_maybe=(\S+)", a or "")
        if mm:
            maybe = f(mm.group(1))
            if maybe is None:
                return 0, 1, 0, "never reached maybe (observer did not notice at all)"
            if not seen:
                return (window_s - maybe if kind == "detect_sneaker" else 0), 1, int(kind == "detect_sneaker"),                     "censored(maybe, not seen)"
            secs = max(secs - maybe, 0.01)
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
        bonus_ev, mods = "", []   # this point's pg_bonus base/vanilla/hooked values and stat condition multipliers
        for i, line in enumerate(lines):
            bm = re.search(r"^PASS\s+\d+\s+pg_bonus .*=> .*base=([\d.]+) vanilla_effective=([\d.]+) effective=([\d.]+)", line)
            if bm:
                bonus_ev = "/".join(bm.groups())
            sm = re.search(r"^PASS\s+\d+\s+stat \S+ \S+ => .* mod=([\d.]+)", line)
            if sm:
                mods.append(sm.group(1))
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
                # game time must advance in a timed window (a paused or stuck clock is not a slow worker)
                if kind in ("craft", "operate", "research", "construct") and (f(win) or 0) <= 0:
                    count, ok, note = 0, 0, "invalid: game clock did not advance (%s)" % win
                cond_ev = (" eff=" + bonus_ev if bonus_ev else "") + (" mod=" + ">".join(mods) if mods else "")
                bonus_ev, mods = "", []
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
                    "%s %s %s %s%s%s%s" % (label, kind, note, base, extra, probe, cond_ev), setup, evidence, prov))
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


def gatecheck(paths):
    """One GATE line per (output file, profession) from csv files made by `csv` on gate outputs (labels g1..g4)."""
    import csv as _csv
    groups = {}
    for path in paths:
        for r in _csv.DictReader(open(path, encoding="utf-8")):
            notes = r["notes"].split()
            if not notes or not re.match(r"^g[1-4]$", notes[0]):
                continue
            src = next((n for n in notes if n.endswith(".out")), path)
            groups.setdefault((src, r["profession"]), {})[notes[0]] = r
    for (src, prof), g in sorted(groups.items()):
        def rate(lb):
            r = g.get(lb)
            if not r or r["valid"] != "1" or float(r["elapsed_game_seconds"] or 0) <= 0:
                return None
            return float(r["result_count"]) / float(r["elapsed_game_seconds"])
        def ev(lb, key):
            m = re.search(r"\b%s=(\S+)" % key, (g.get(lb) or {}).get("notes", ""))
            return m.group(1) if m else ""
        rates = dict((lb, rate(lb)) for lb in ("g1", "g2", "g3", "g4"))
        mods = [float(ev(lb, "mod").split(">")[0]) for lb in rates if ev(lb, "mod")]
        eff = dict((lb, ev(lb, "eff")) for lb in rates)
        kind = ((g.get("g1") or {}).get("notes", "x x").split() + ["", ""])[1]
        head = "GATE %s %s" % (prof, os.path.basename(src))
        bad = [lb for lb, v in rates.items() if v is None]
        if bad:
            why = "; ".join("%s %s" % (lb, (g.get(lb) or {}).get("notes", "missing")[:80]) for lb in bad)
            print("%s FAIL setup: invalid/missing %s" % (head, why))
            continue
        r1, r2, r3, r4 = (rates[x] for x in ("g1", "g2", "g3", "g4"))
        base = (r1 + r4) / 2.0
        if base <= 0:
            print("%s FAIL measurement: zero result at low skill (%.4g/%.4g)" % (head, r1, r4))
            continue
        drift = abs(r1 - r4) / base
        noise = max(0.01 if kind == "chance" else 0.03, drift)
        skill = r3 / base - 1.0
        gear = r2 / base - 1.0
        # predicted gear gain from the game's own hooked values: (eff_g2 - eff_g1) / (eff_g3 - eff_g1) of the skill gain
        try:
            e1, e2, e3 = (float(eff[x].split("/")[2]) for x in ("g1", "g2", "g3"))
            frac = (e2 - e1) / (e3 - e1) if e3 != e1 else 0.0
            mech = "eff %.3g->%.3g (x%.2f)" % (e1, e2, e2 / e1 if e1 else 0)
        except (ValueError, IndexError):
            frac, mech = (GATE_LOW * 0.5) / (GATE_HIGH - GATE_LOW), "eff ?"
        pred = frac * skill
        if kind == "chance" and prof.split("_")[0] == "lockpicking" and mech != "eff ?":
            # the game's lockpick chance is exponential, not linear: 0.9 / 2^((level - skill) * 0.1), capped at 0.9
            # (harness BalanceCommands.inc, DoorLock chance via getStat), so gear adding d effective points multiplies
            # the chance by 2^(d / 10) up to the cap (m34: 88 s25 0.0281 -> 0.0669 = x2^1.25 exactly)
            pred = min(0.9, base * 2.0 ** ((e2 - e1) / 10.0)) / base - 1.0
            mech += " exp"
        if kind == "operate":  # JobOperateScaling multiplies work by the bonus on top of the skill read
            pred = (1 + pred) * 1.5 - 1
        modtxt = "mod " + "/".join("%.3g" % m for m in mods) if mods else "mod ?"
        nums = "s25=%.4g s25+g=%.4g s90=%.4g s25b=%.4g drift=%.1f%% skill=%+.1f%% gear=%+.1f%% pred=%+.1f%% %s %s" % (
            r1, r2, r3, r4, 100 * drift, 100 * skill, 100 * gear, 100 * pred, mech, modtxt)
        if mods and max(mods) > 1.10 * min(mods):
            verdict = "FAIL measurement: conditions changed between points (%s)" % modtxt
        elif abs(skill) <= 2 * noise:
            verdict = "FAIL flat: skill 25 vs 90 within noise (measurement or game behaviour)"
        elif skill < 0:  # m31 (coordinator): a skill response in the wrong direction is never a PASS
            verdict = "FAIL skill: negative response (s90 worse than s25: measurement or game behaviour)"
        elif abs(pred) <= noise:
            verdict = "PASS skill; gear unresolved (predicted gain within noise)"
        elif gear >= 0.5 * pred - noise and gear > noise:
            verdict = "PASS"
        else:
            verdict = "FAIL gear: no measurable gain where the skill curve predicts one"
        print("%s %s %s" % (head, verdict, nums))


def list_files():
    for p in P:
        print("%-42s rows %-8s kind %s" % (p["file"], p["rows"], p["kind"]))


if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1] == "gen":
        gen()
    elif len(sys.argv) >= 2 and sys.argv[1] == "gate":
        GATE = True
        gen()
    elif len(sys.argv) >= 3 and sys.argv[1] == "gatecheck":
        gatecheck(sys.argv[2:])
    elif len(sys.argv) >= 2 and sys.argv[1] == "list":
        list_files()
    elif len(sys.argv) >= 3 and sys.argv[1] == "csv":
        to_csv(sys.argv[2:])
    else:
        print(__doc__)
        sys.exit(2)
