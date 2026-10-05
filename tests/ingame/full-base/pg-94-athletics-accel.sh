#!/usr/bin/env bash
# pg-94-athletics-accel.sh (WSL, PG 254 / Shay D4): runs pg-94-athletics-accel.txt (game in the world on a Full-Base kah
# copy, Beaks + Avarek) and turns its ACC lines (a1/a3 pg_accel off, a2/a4 on) into RESULT lines. pg_accel is put
# back on at the end either way. Means of the two points per side:
# RESULT 254-accel-start:    t90 on <= 0.85 x t90 off (expected ~0.67: acceleration x1.5)
# RESULT 254-accel-stop:     stop_dist on <= 0.85 x stop_dist off (expected ~0.67: braking x1.5)
# RESULT 254-accel-topspeed: cruise_speed and runspeed movement_speed on within 5% of off (top speed unchanged),
#                            and pg_accel Beaks reported scale=1.5 handle_match=1 scaled>0
set -u
DIR=$(cd "$(dirname "$0")" && pwd)
SC=$DIR/pg-94-athletics-accel.txt
OUT=${RESULT_LOG:-/tmp/pg-94-athletics-accel.log}
stobe-auto run "$SC" --csv "${OUT%.log}.csv" > "$OUT" 2>&1
stobe-auto pg_accel on >/dev/null 2>&1
STEPS=$(grep -oE '^== [0-9]+ passed, [0-9]+ failed' "$OUT" | tail -1)
FAILED=$(echo "$STEPS" | awk '{print $4}'); FAILED=${FAILED:-?}
# field <key> <on|off>: mean of that key over the ACC lines of one side ("" if any point lacks a number)
field() { grep -oE "ACC a[0-9] $2 ms=.*" "$OUT" | awk -v k="$1" '{ok=0; for(i=1;i<=NF;i++){split($i,kv,"="); if(kv[1]==k && kv[2] ~ /^[0-9.]+$/){s+=kv[2]; ok=1}} n++; if(!ok) bad=1}
  END{ if(n==2 && !bad) printf "%.4f", s/n }'; }
HOOK=$(grep -oE 'ACCHOOK a[0-9] scaled=[0-9]+ npc=Beaks accel=[0-9.]+ athletics_pct=[0-9.]+ scale=[0-9.]+ handle_match=[0-9]' "$OUT" | tail -1 | cut -d' ' -f3-)
tail_log() { [ "$1" = FAIL ] && echo " log=$OUT"; }
ratio_row() { # row key max_ratio
  local on off V
  on=$(field "$2" on); off=$(field "$2" off)
  if [ -n "$on" ] && [ -n "$off" ]; then
    V=$(awk -v a="$on" -v b="$off" -v m="$3" 'BEGIN{r=(b>0)?a/b:9; printf "%s %.3f", (r<=m)?"PASS":"FAIL", r}')
    echo "RESULT $1 ${V%% *} $2 on=$on off=$off ratio=${V#* } (want <= $3, expected ~0.67) steps_failed=$FAILED$(tail_log "${V%% *}")"
  else
    echo "RESULT $1 FAIL missing data: $2 on=${on:-none} off=${off:-none} (2 points each side) steps_failed=$FAILED log=$OUT"
  fi
}
ratio_row 254-accel-start t90 0.85
ratio_row 254-accel-stop stop_dist 0.85
CON=$(field cruise_speed on); COF=$(field cruise_speed off); MON=$(field ms on); MOF=$(field ms off)
if [ -n "$CON" ] && [ -n "$COF" ] && [ -n "$MON" ] && [ -n "$MOF" ] && [ -n "$HOOK" ]; then
  V=$(awk -v a="$CON" -v b="$COF" -v c="$MON" -v d="$MOF" -v h="$HOOK" 'BEGIN{r=a/b; q=c/d;
    hk=(h ~ /scale=1\.5 / && h ~ /handle_match=1/ && h !~ /scaled=0 /);
    printf "%s cruise_ratio=%.3f runspeed_ratio=%.3f", (r>=0.95&&r<=1.05&&q>=0.95&&q<=1.05&&hk)?"PASS":"FAIL", r, q}')
  echo "RESULT 254-accel-topspeed ${V%% *} ${V#* } cruise on=$CON off=$COF movement_speed on=$MON off=$MOF $HOOK steps_failed=$FAILED$(tail_log "${V%% *}")"
else
  echo "RESULT 254-accel-topspeed FAIL missing data: cruise on=${CON:-none} off=${COF:-none} ms on=${MON:-none} off=${MOF:-none} hook=${HOOK:-none} steps_failed=$FAILED log=$OUT"
fi
