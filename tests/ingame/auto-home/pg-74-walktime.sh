#!/usr/bin/env bash
# pg-74-walktime.sh (WSL, PG 254): the +x path from the auto-home spot can be unreachable (m36 batch Y: 300 units
# +x never started). Probe +x/-x/+z/-z with one 300-unit run each (harness 05fd291: an unreachable target fails
# after 20 s), then run pg-74-walktime.txt along the first axis Malzin really ran. Output: the scenario's own lines.
set -u
DIR=$(cd "$(dirname "$0")" && pwd)
SC=$DIR/pg-74-walktime.txt
OUT=${RESULT_LOG:-/tmp/pg-74-walktime.txt}
TMP=$(mktemp /tmp/pg-74-XXXX.txt)
stobe-auto speed 1 >/dev/null
AX=
for a in +x -x +z -z; do
  stobe-auto teleport Malzin Shay dist 3 >/dev/null; sleep 2
  r=$(stobe-auto walktime Malzin 300 $a run 2>&1)
  echo "probe $a: $r" | cut -c1-220
  case "$r" in *" walked "*) AX=$a; break;; esac
done
if [ -z "$AX" ]; then echo "RESULT pg-74-walktime FAIL setup: no reachable axis from the home spot"; exit 1; fi
sed "s/walktime Malzin 300 +x run/walktime Malzin 300 $AX run/" "$SC" > "$TMP"
echo "axis=$AX"
stobe-auto run "$TMP" --csv "${OUT%.txt}.csv"
rc=$?
rm -f "$TMP"
exit $rc
