$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkg = Join-Path $root "out\package\ProfessionGearProgression"
$required = @(
  "ProfessionGearProgression.dll",
  "ProfessionGearProgression.mod",
  "RE_Kenshi.json",
  "mod.info",
  "ProfessionGear.ini",
  "ProfessionGear.rules",
  "LIVE_TEST_RUNBOOK.md"
)
$missing=@()
foreach($name in $required){
  $p=Join-Path $pkg $name
  if(-not (Test-Path $p)){ $missing += $name; Write-Host "MISSING $name" }
  else { Write-Host "OK      $name" }
}
if($missing.Count){ throw ("Package verification failed: "+$missing.Count+" missing file(s)") }
$mod=(Get-Item (Join-Path $pkg "ProfessionGearProgression.mod")).Length
if($mod -le 0){ throw "ProfessionGearProgression.mod is empty" }
$runtimeForbidden=@("ProfessionGear.log","profession_gear_affixes.tsv")
foreach($name in $runtimeForbidden){
  if(Test-Path (Join-Path $pkg $name)){ throw "Runtime state leaked into package: $name" }
}
Write-Host ("Package verification passed: "+$required.Count+" required files")
