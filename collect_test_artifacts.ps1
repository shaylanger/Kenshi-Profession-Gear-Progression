param(
  [string]$KenshiRoot = "D:\Steam\steamapps\common\Kenshi",
  [string]$Label = "manual"
)
$ErrorActionPreference="Stop"
$repo=Split-Path -Parent $MyInvocation.MyCommand.Path
$stamp=Get-Date -Format "yyyyMMdd-HHmmss"
$safe=($Label -replace '[^A-Za-z0-9._-]','_')
$out=Join-Path $repo ("out\test-runs\"+$stamp+"-"+$safe)
New-Item -ItemType Directory -Force -Path $out | Out-Null
$pg=Join-Path $KenshiRoot "mods\ProfessionGearProgression"
foreach($name in @("ProfessionGear.log","profession_gear_affixes.tsv","ProfessionGear.ini")){
  $p=Join-Path $pg $name
  if(Test-Path $p){ Copy-Item $p $out -Force }
}
foreach($pair in @(
  @("Stobe","Stobe.log"),
  @("KenshiFP","KenshiFP.log")
)){
  $p=Join-Path $KenshiRoot ("mods\"+$pair[0]+"\"+$pair[1])
  if(Test-Path $p){ Copy-Item $p $out -Force }
}
Write-Host ("Collected available test artifacts to: "+$out)
