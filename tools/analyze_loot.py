#!/usr/bin/env python3
"""World-loot distribution (TEST_PLAN row 250) from ProfessionGear.log (NormalVerbose: every roll is logged).

Reads the `roll source=world_loot ... tier=<t> tags=<a,b> affixes=<Stat:pct,...|none>` lines and prints:
the rolled share overall and per tier against the design (half the tier chance), the stat mix, and a tag x stat
table (coherence: every affix stat should come from the item's own tags). Verdict for row 250:
  share of eligible items with an affix in 0.15..0.60, >= 4 different stats, no stat above 50% of the affixes.

Usage: python3 tools/analyze_loot.py ProfessionGear.log [more logs...]
"""
import collections
import re
import sys

TIER_CHANCE = [.20, .30, .45, .60, .75, .88, .96]   # Normal rules; world loot rolls at x0.5
LINE = re.compile(r'roll source=world_loot .*?name="([^"]*)".*? tier=(\d+) tags=(\S*) affixes=(\S+)')


def main(paths):
    rows = []
    for path in paths:
        for line in open(path, encoding="utf-8", errors="replace"):
            m = LINE.search(line)
            if m:
                name, tier, tags, affixes = m.groups()
                stats = [] if affixes == "none" else [a.split(":")[0] for a in affixes.split(",")]
                rows.append((name, int(tier), [t for t in tags.split(",") if t], stats))
    if not rows:
        print("no `roll source=world_loot` lines (NormalVerbose mode needed)")
        return 1
    n = len(rows)
    rolled = [r for r in rows if r[3]]
    share = len(rolled) / n
    print("world-loot rolls: %d items, %d with an affix (share %.2f)" % (n, len(rolled), share))
    print("\nper tier: items / rolled / share / design (0.5 x tier chance)")
    by_tier = collections.defaultdict(list)
    for r in rows:
        by_tier[r[1]].append(r)
    expected_total = 0.0
    for t in sorted(by_tier):
        lst = by_tier[t]
        k = sum(1 for r in lst if r[3])
        exp = 0.5 * TIER_CHANCE[t] if 0 <= t < len(TIER_CHANCE) else float("nan")
        expected_total += exp * len(lst)
        print("  tier %d: %4d %4d  %.2f  %.2f" % (t, len(lst), k, k / len(lst), exp))
    print("  expected share for this tier mix: %.2f" % (expected_total / n))
    stats = collections.Counter(s for r in rows for s in r[3])
    total_aff = sum(stats.values())
    print("\nstat mix (%d affixes):" % total_aff)
    for s, c in stats.most_common():
        print("  %-18s %4d  %.0f%%" % (s, c, 100.0 * c / total_aff))
    print("\ntag x stat (coherence):")
    cross = collections.defaultdict(collections.Counter)
    for r in rolled:
        for t in r[2] or ["(no tag)"]:
            for s in r[3]:
                cross[t][s] += 1
    for t in sorted(cross):
        print("  %-22s %s" % (t, ", ".join("%s=%d" % kv for kv in cross[t].most_common())))
    print("\nitems by name (rolled/all):")
    names = collections.defaultdict(lambda: [0, 0])
    for r in rows:
        names[r[0]][1] += 1
        names[r[0]][0] += bool(r[3])
    for name, (k, a) in sorted(names.items(), key=lambda kv: -kv[1][1])[:30]:
        print("  %-30s %d/%d" % (name, k, a))
    top = stats.most_common(1)[0][1] / total_aff if total_aff else 1.0
    ok = 0.15 <= share <= 0.60 and len(stats) >= 4 and top <= 0.5
    print("\nRow 250 verdict: %s (share %.2f in 0.15..0.60, %d stats >= 4, top stat %.0f%% <= 50%%)"
          % ("PASS" if ok else "FAIL", share, len(stats), 100 * top))
    return 0 if ok else 2


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1:]))
