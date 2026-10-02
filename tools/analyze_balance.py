#!/usr/bin/env python3
"""
Analyze Profession Gear balance calibration CSV data.

Expected columns:
test_id,profession,base_skill,effective_bonus_pct,elapsed_game_seconds,result_count,valid

Optional:
character,gear_loadout,tier,roll_pct,notes

Throughput = result_count / elapsed_game_seconds.
Gain is measured relative to the same profession/base_skill baseline where
effective_bonus_pct == 0.
"""
import argparse
import csv
import math
import statistics
from collections import defaultdict

TARGETS = {
    "single_shoddy": (2.0, 5.0),
    "full_shoddy": (10.0, 15.0),
    "full_mid": (15.0, 25.0),
    "full_high": (25.0, 35.0),
    "full_top": (35.0, 50.0),
}

def f(row, key, default=0.0):
    try:
        return float(row.get(key, default))
    except (TypeError, ValueError):
        return default

def valid(row):
    v = str(row.get("valid", "1")).strip().lower()
    return v not in ("0", "false", "no", "invalid")

def median(values):
    return statistics.median(values) if values else float("nan")

def mean(values):
    return statistics.mean(values) if values else float("nan")

def stdev(values):
    return statistics.stdev(values) if len(values) > 1 else 0.0

def nearest_bonus(points, target_gain):
    if not points:
        return None
    return min(points, key=lambda p: abs(p["gain_pct"] - target_gain))

def candidate_band(points, low_target, high_target):
    usable = sorted(
        [p for p in points if not math.isnan(p["gain_pct"])],
        key=lambda p: p["gain_pct"]
    )
    if not usable:
        return None, "no-data"
    if usable[-1]["gain_pct"] < low_target:
        return None, "insufficient-high"
    if usable[0]["gain_pct"] > high_target:
        return None, "insufficient-low"
    low = nearest_bonus(usable, low_target)
    high = nearest_bonus(usable, high_target)
    return (
        min(low["effective_bonus_pct"], high["effective_bonus_pct"]),
        max(low["effective_bonus_pct"], high["effective_bonus_pct"])
    ), "ok"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv_path")
    ap.add_argument("--summary-csv")
    args = ap.parse_args()

    rows = []
    with open(args.csv_path, newline="", encoding="utf-8-sig") as fh:
        for row in csv.DictReader(fh):
            if not valid(row):
                continue
            elapsed = f(row, "elapsed_game_seconds")
            count = f(row, "result_count", 1.0)
            if elapsed <= 0 or count <= 0:
                continue
            row["_skill"] = f(row, "base_skill")
            row["_bonus"] = f(row, "effective_bonus_pct")
            row["_throughput"] = count / elapsed
            rows.append(row)

    grouped = defaultdict(list)
    for row in rows:
        grouped[(row.get("profession","").strip(), row["_skill"], row["_bonus"])].append(row["_throughput"])

    baseline = {}
    for (profession, skill, bonus), vals in grouped.items():
        if abs(bonus) < 1e-9:
            baseline[(profession, skill)] = median(vals)

    summary = []
    for (profession, skill, bonus), vals in sorted(grouped.items()):
        base = baseline.get((profession, skill))
        med = median(vals)
        gain = ((med / base) - 1.0) * 100.0 if base and base > 0 else float("nan")
        summary.append({
            "profession": profession,
            "base_skill": skill,
            "effective_bonus_pct": bonus,
            "samples": len(vals),
            "throughput_median": med,
            "throughput_mean": mean(vals),
            "throughput_stdev": stdev(vals),
            "gain_pct": gain,
        })

    print("Profession Gear balance analysis")
    print("=" * 72)
    professions = sorted(set(x["profession"] for x in summary))
    for profession in professions:
        print("\n" + profession)
        print("-" * len(profession))
        skills = sorted(set(x["base_skill"] for x in summary if x["profession"] == profession))
        for skill in skills:
            pts = [x for x in summary if x["profession"] == profession and x["base_skill"] == skill]
            if not any(not math.isnan(x["gain_pct"]) for x in pts):
                print(f"  skill {skill:g}: missing 0% baseline")
                continue
            print(f"  skill {skill:g}:")
            for p in pts:
                gain = "n/a" if math.isnan(p["gain_pct"]) else f"{p['gain_pct']:+.2f}%"
                print(
                    f"    bonus {p['effective_bonus_pct']:>5.1f}%  "
                    f"gain {gain:>9}  n={p['samples']}  "
                    f"throughput={p['throughput_median']:.6f}"
                )

        skill50 = [x for x in summary if x["profession"] == profession and abs(x["base_skill"] - 50.0) < 0.001 and not math.isnan(x["gain_pct"])]
        if skill50:
            print("  candidate effective-skill bonuses at base skill 50:")
            for name, (lo, hi) in TARGETS.items():
                band, status = candidate_band(skill50, lo, hi)
                if status == "ok":
                    low_bonus, high_bonus = band
                    print(
                        f"    {name:14s}: ~{low_bonus:g}% to {high_bonus:g}% "
                        f"effective-skill bonus for {lo:g}-{hi:g}% target throughput"
                    )
                elif status == "insufficient-high":
                    max_gain = max(p["gain_pct"] for p in skill50)
                    max_bonus = max(p["effective_bonus_pct"] for p in skill50)
                    print(
                        f"    {name:14s}: insufficient sweep; highest tested "
                        f"{max_bonus:g}% bonus produced {max_gain:.2f}% throughput gain"
                    )
                else:
                    print(f"    {name:14s}: insufficient data")

    if args.summary_csv:
        fields = [
            "profession","base_skill","effective_bonus_pct","samples",
            "throughput_median","throughput_mean","throughput_stdev","gain_pct"
        ]
        with open(args.summary_csv, "w", newline="", encoding="utf-8") as fh:
            w = csv.DictWriter(fh, fieldnames=fields)
            w.writeheader()
            for row in summary:
                w.writerow(row)
        print(f"\nWrote {args.summary_csv}")

if __name__ == "__main__":
    main()
