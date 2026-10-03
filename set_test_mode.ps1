param(
  [ValidateSet("Forced","Normal")]
  [string]$Mode,
  [string]$KenshiRoot = "D:\Steam\steamapps\common\Kenshi"
)
$ErrorActionPreference="Stop"
$repo=Split-Path -Parent $MyInvocation.MyCommand.Path
$dest=Join-Path $KenshiRoot "mods\ProfessionGearProgression\ProfessionGear.ini"
if(-not (Test-Path (Split-Path -Parent $dest))){ throw "ProfessionGearProgression is not installed: $dest" }
if($Mode -eq "Forced"){
  $src=Join-Path $repo "tests\fixtures\ProfessionGear.forced-test.ini"
}else{
  $src=Join-Path $repo "mod\ProfessionGear.ini"
}
Copy-Item $src $dest -Force
Write-Host ("ProfessionGear test mode set to: "+$Mode)
Write-Host ("Config: "+$dest)
