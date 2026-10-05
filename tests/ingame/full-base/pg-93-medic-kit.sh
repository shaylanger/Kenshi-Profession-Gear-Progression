#!/usr/bin/env bash
# pg-93-medic-kit.sh (WSL, PG 191 / Shay D2): runs pg-93-medic-kit.txt (game in the world on a Full-Base kah copy,
# Beaks + Avarek) and turns its MK lines into RESULT lines. pg_medickit is put back on at the end either way.
# RESULT 191-medickit-90: p1 (Medic 90 own50, kit scaling on) bandage_rate >= 1.25 x p2 (same, off), p2 within 15% of
#   p3 (Medic 90 no gear: the kit caps both), and pg_medickit counted scaled calls with scaled = 1.5 x quality.
# RESULT 191-medickit-25: p4 (Medic 25 own50, on) within 10% of p5 (same, off): skill under the cap, no double dip.
set -u
DIR=$(cd "$(dirname "$0")" && pwd)
SC=$DIR/pg-93-medic-kit.txt
OUT=${RESULT_LOG:-/tmp/pg-93-medic-kit.log}
stobe-auto run "$SC" --csv "${OUT%.log}.csv" > "$OUT" 2>&1
stobe-auto pg_medickit on >/dev/null 2>&1
STEPS=$(grep -oE '^== [0-9]+ passed, [0-9]+ failed' "$OUT" | tail -1)
FAILED=$(echo "$STEPS" | awk '{print $4}'); FAILED=${FAILED:-?}
v() { grep -oE "MK $1 [0-9.]+" "$OUT" | tail -1 | awk '{print $3}'; }
P1=$(v p1); P2=$(v p2); P3=$(v p3); P4=$(v p4); P5=$(v p5)
CALLS=$(grep -oE 'MKCALLS p1 calls=[0-9]+ last_quality=[0-9.]+ last_scaled=[0-9.]+' "$OUT" | tail -1 | cut -d' ' -f3-)
num() { [[ "$1" =~ ^[0-9]+(\.[0-9]+)?$ ]]; }
if num "$P1" && num "$P2" && num "$P3" && [ -n "$CALLS" ]; then
  Q=$(echo "$CALLS" | sed -E 's/.*last_quality=([0-9.]+).*/\1/'); S=$(echo "$CALLS" | sed -E 's/.*last_scaled=([0-9.]+).*/\1/')
  V=$(awk -v a="$P1" -v b="$P2" -v c="$P3" -v q="$Q" -v s="$S" 'BEGIN{r=(b>0)?a/b:0; d=(c>0)?b/c:0;
    ok=(r>=1.25 && d>=0.85 && d<=1.15 && q>0 && s/q>1.49 && s/q<1.51); printf "%s ratio_on_off=%.3f off_vs_none=%.3f", ok?"PASS":"FAIL", r, d}')
  echo "RESULT 191-medickit-90 ${V%% *} ${V#* } rate on=$P1 off=$P2 none=$P3 $CALLS steps_failed=$FAILED$([ "${V%% *}" = FAIL ] && echo " log=$OUT")"
else
  echo "RESULT 191-medickit-90 FAIL missing data: on=${P1:-none} off=${P2:-none} none=${P3:-none} calls=${CALLS:-none} steps_failed=$FAILED log=$OUT"
fi
if num "$P4" && num "$P5"; then
  V=$(awk -v a="$P4" -v b="$P5" 'BEGIN{r=(b>0)?a/b:0; printf "%s ratio_on_off=%.3f", (r>=0.9&&r<=1.1)?"PASS":"FAIL", r}')
  echo "RESULT 191-medickit-25 ${V%% *} ${V#* } rate on=$P4 off=$P5 steps_failed=$FAILED$([ "${V%% *}" = FAIL ] && echo " log=$OUT")"
else
  echo "RESULT 191-medickit-25 FAIL missing data: on=${P4:-none} off=${P5:-none} steps_failed=$FAILED log=$OUT"
fi
