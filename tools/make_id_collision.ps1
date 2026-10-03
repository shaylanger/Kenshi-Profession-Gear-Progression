<#
make_id_collision.ps1 -Key <pgp id> | -Restore   (PG row 274, TEST ONLY; Kenshi must be closed)

Makes the "existing sidecar key reused by a different base item" case: the sidecar row of <Key>
(an item that a kah-* test save carries, see tests/ingame/auto-home/pg-57a-id-collision-setup.txt)
gets another item's base ID and a marker affix (stat 3 at 77%). On the next load of that save the
item arrives with <Key>, PG finds a record for a different base item and must rebind + reroll
(log line "persistent-id collision/reuse: rebound ...").

Only the sidecar in the installed mod folder changes; no save file, no fixture master is touched.
The original row is kept in profession_gear_affixes.tsv.collision-backup; -Restore puts it back.
#>
param(
  [string]$Key,
  [switch]$Restore,
  [string]$ModDir = "D:\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression"
)
$ErrorActionPreference = "Stop"
if (Get-Process kenshi_x64 -ErrorAction SilentlyContinue) { throw "Kenshi is running: close it first (the sidecar is read at startup and rewritten while playing)" }
$db = Join-Path $ModDir "profession_gear_affixes.tsv"
$bak = Join-Path $ModDir "profession_gear_affixes.tsv.collision-backup"
$lines = [System.IO.File]::ReadAllLines($db)

if ($Restore) {
  if (-not (Test-Path $bak)) { throw "no backup row: $bak" }
  $orig = [System.IO.File]::ReadAllText($bak).TrimEnd("`r", "`n")
  $k = $orig.Split("`t")[0]
  $found = $false
  for ($i = 0; $i -lt $lines.Length; $i++) {
    if ($lines[$i].Split("`t")[0] -eq $k) { $lines[$i] = $orig; $found = $true }
  }
  if (-not $found) { $lines += $orig }
  [System.IO.File]::WriteAllLines($db, $lines)
  Remove-Item $bak
  Write-Host "restored row $k"
  return
}

if (-not $Key) { throw "usage: make_id_collision.ps1 -Key <pgp id> | -Restore" }
if (Test-Path $bak) { throw "a collision is already in place ($bak): run -Restore first" }
$idx = -1
for ($i = 0; $i -lt $lines.Length; $i++) { if ($lines[$i].Split("`t")[0] -eq $Key) { $idx = $i } }
if ($idx -lt 0) { throw "key not in the sidecar: $Key (was the save made and Kenshi closed normally?)" }
$f = $lines[$idx].Split("`t")
if ($f.Length -lt 4) { throw "unexpected row format: $($lines[$idx])" }
$other = $null
foreach ($l in $lines) {
  if ($l.StartsWith("#")) { continue }
  $g = $l.Split("`t")
  if ($g.Length -ge 4 -and $g[1] -ne $f[1]) { $other = $g[1]; break }
}
if (-not $other) { throw "no row with a different base ID to borrow" }
[System.IO.File]::WriteAllText($bak, $lines[$idx] + "`n")
$lines[$idx] = "$($f[0])`t$other`t$($f[2])`t3:77"
[System.IO.File]::WriteAllLines($db, $lines)
Write-Host "row $Key : base $($f[1]) -> $other, affixes $($f[3]) -> 3:77 (marker). Original kept in $bak"
