#!/usr/bin/env bash
# pg-94-athletics-accel.sh (WSL, PG 254 / Shay D4): runs pg-94-athletics-accel.txt (game in the world on a Full-Base kah
# copy, Beaks + Avarek) and turns its ACC lines (odd a1-a11 pg_accel off, even a2-a12 on) into RESULT lines. pg_accel is put
# back on at the end either way. Medians of the 6 points per side:
# RESULT 254-accel-start:    t90 on <= 0.85 x t90 off (expected ~0.67: acceleration x1.5)
# RESULT 254-accel-stop:     stop_dist on <= 0.85 x stop_dist off (expected ~0.67: braking x1.5)
# RESULT 254-accel-topspeed: cruise_speed and runspeed max_speed on within 5% of off (top speed unchanged),
#                            and pg_accel Beaks reported scale=1.5 handle_match=1 scaled>0
# RESULT 254-accel-onscreen: setup gate. Kenshi runs the acceleration physics only while he is on screen, so every
#                            acceltime uses follow and each of the 12 points must report follow=1 onscreen_frames>0
#                            offscreen_frames=0; otherwise the point is an invalid setup and every row above FAILs on it
set -u
DIR=$(cd "$(dirname "$0")" && pwd)
SC=$DIR/pg-94-athletics-accel.txt
OUT=${RESULT_LOG:-/tmp/pg-94-athletics-accel.log}
stobe-auto run "$SC" --csv "${OUT%.log}.csv" > "$OUT" 2>&1
stobe-auto pg_accel on >/dev/null 2>&1
STEPS=$(grep -oE '^== [0-9]+ passed, [0-9]+ failed' "$OUT" | tail -1)
FAILED=$(echo "$STEPS" | awk '{print $4}'); FAILED=${FAILED:-?}
# the run log truncates long lines (offscreen_frames fell off in m43): read the echoed values from the CSV
DATA=${OUT%.log}.acc; cut -d, -f4- "${OUT%.log}.csv" 2>/dev/null > "$DATA"
# field <key> <on|off>: median of that key over the ACC lines of one side ("" unless all 6 points have a number)
field() { grep -oE "ACC a[0-9]+ $2 ms=.*" "$DATA" | awk -v k="$1" '{ok=0; for(i=1;i<=NF;i++){split($i,kv,"="); if(kv[1]==k && kv[2] ~ /^[0-9.]+$/){v[++m]=kv[2]+0; ok=1}} n++; if(!ok) bad=1}
  END{ if(n==6 && !bad){ for(i=1;i<=m;i++) for(j=i+1;j<=m;j++) if(v[j]<v[i]){x=v[i];v[i]=v[j];v[j]=x}; printf "%.4f", (v[3]+v[4])/2 } }'; }
HOOK=$(grep -oE 'ACCHOOK a[0-9]+ scaled=[0-9]+ npc=Beaks accel=[0-9.]+ athletics_pct=[0-9.]+ scale=[0-9.]+ handle_match=[0-9]' "$DATA" | tail -1 | cut -d' ' -f3-)
tail_log() { [ "$1" = FAIL ] && echo " log=$OUT"; }
# on-screen gate: points without follow=1, with offscreen_frames>0 or with no on-screen frames (harness b159c4c+)
GATE=$(grep -oE "ACC a[0-9]+ (on|off) ms=.*" "$DATA" | awk '{f="";on="";off=""; for(i=1;i<=NF;i++){split($i,kv,"=");
    if(kv[1]=="follow")f=kv[2]; if(kv[1]=="onscreen_frames")on=kv[2]; if(kv[1]=="offscreen_frames")off=kv[2]}
  n++; if(f!="1" || on=="" || off=="" || off+0>0 || on+0==0){bad++; list=list sprintf(" %s(follow=%s on=%s off=%s)",$2,(f==""?"?":f),(on==""?"?":on),(off==""?"?":off))}
  else {ton+=on}}
  END{printf "%d %d %d%s", n, bad, ton, list}')
read -r GN GBAD GON GLIST <<< "$GATE"
GN=${GN:-0}; GBAD=${GBAD:-0}
if [ "$GN" -eq 12 ] && [ "$GBAD" -eq 0 ]; then
  INVALID=""
  echo "RESULT 254-accel-onscreen PASS 12/12 points on screen for the whole run (offscreen_frames=0, onscreen_frames total=$GON)"
else
  INVALID="invalid setup: $GBAD of $GN points (want 12) not fully on screen:${GLIST:- none}"
  echo "RESULT 254-accel-onscreen FAIL $INVALID (off-screen movement skips the acceleration physics) steps_failed=$FAILED log=$OUT"
fi
ratio_row() { # row key max_ratio
  local on off V
  on=$(field "$2" on); off=$(field "$2" off)
  if [ -n "$INVALID" ]; then
    echo "RESULT $1 FAIL $INVALID ($2 on=${on:-none} off=${off:-none} not judged) log=$OUT"
  elif [ -n "$on" ] && [ -n "$off" ]; then
    V=$(awk -v a="$on" -v b="$off" -v m="$3" 'BEGIN{r=(b>0)?a/b:9; printf "%s %.3f", (r<=m)?"PASS":"FAIL", r}')
    echo "RESULT $1 ${V%% *} $2 on=$on off=$off ratio=${V#* } (want <= $3, expected ~0.67) steps_failed=$FAILED$(tail_log "${V%% *}")"
  else
    echo "RESULT $1 FAIL missing data: $2 on=${on:-none} off=${off:-none} (6 points each side) steps_failed=$FAILED log=$OUT"
  fi
}
ratio_row 254-accel-start t90 0.85
# m41: t90 barely moved (0.96) while t50 fell 27%: report the early ramp too (t90 assertion unchanged)
ratio_row 254-accel-start50 t50 0.85
ratio_row 254-accel-stop stop_dist 0.85
CON=$(field cruise_speed on); COF=$(field cruise_speed off); MON=$(field ms on); MOF=$(field ms off)
if [ -n "$INVALID" ]; then
  echo "RESULT 254-accel-topspeed FAIL $INVALID (cruise on=${CON:-none} off=${COF:-none} not judged) log=$OUT"
elif [ -n "$CON" ] && [ -n "$COF" ] && [ -n "$MON" ] && [ -n "$MOF" ] && [ -n "$HOOK" ]; then
  V=$(awk -v a="$CON" -v b="$COF" -v c="$MON" -v d="$MOF" -v h="$HOOK" 'BEGIN{r=(b>0)?a/b:9; q=(d>0)?c/d:9;
    hk=(h ~ /scale=1\.5 / && h ~ /handle_match=1/ && h !~ /scaled=0 /);
    printf "%s cruise_ratio=%.3f runspeed_ratio=%.3f", (r>=0.95&&r<=1.05&&q>=0.95&&q<=1.05&&hk)?"PASS":"FAIL", r, q}')
  echo "RESULT 254-accel-topspeed ${V%% *} ${V#* } cruise on=$CON off=$COF max_speed on=$MON off=$MOF $HOOK steps_failed=$FAILED$(tail_log "${V%% *}")"
else
  echo "RESULT 254-accel-topspeed FAIL missing data: cruise on=${CON:-none} off=${COF:-none} ms on=${MON:-none} off=${MOF:-none} hook=${HOOK:-none} steps_failed=$FAILED log=$OUT"
fi
