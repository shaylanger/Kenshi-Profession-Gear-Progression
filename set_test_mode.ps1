param(
  # Config (ProfessionGear.ini):
  #   Forced          every eligible roll succeeds, verbose log (main in-game test runs)
  #   Normal          shipped config
  #   NormalVerbose   shipped chances + roll logging (distribution / rarity runs)
  #   AutoClassifyOff tests 149 + 244 (AutoClassify=false, WorldLootMultiplier=0)
  #   Disabled        test 150 (Enabled=false)
  [ValidateSet("Forced","Normal","NormalVerbose","AutoClassifyOff","Disabled","Keep")]
  [string]$Mode = "Keep",
  # Rules (ProfessionGear.rules): InGameTest = tests\fixtures\ProfessionGear.ingame-test.rules
  [ValidateSet("InGameTest","Normal","Keep")]
  [string]$Rules = "Keep",
  # Sidecar (profession_gear_affixes.tsv), tests 126-128:
  #   StartupTest  set the current sidecar aside and install the corrupt/duplicate/legacy fixture
  #   Empty        set it aside and install an empty file
  #   Restore      put the set-aside sidecar back
  [ValidateSet("StartupTest","Empty","Restore","Keep")]
  [string]$Sidecar = "Keep",
  [string]$KenshiRoot = "D:\Steam\steamapps\common\Kenshi"
)
$ErrorActionPreference="Stop"
$repo=Split-Path -Parent $MyInvocation.MyCommand.Path
$fx=Join-Path $repo "tests\fixtures"
$modDir=Join-Path $KenshiRoot "mods\ProfessionGearProgression"
if(-not (Test-Path $modDir)){ throw "ProfessionGearProgression is not installed: $modDir" }
if(Get-Process kenshi_x64 -ErrorAction SilentlyContinue){ throw "Kenshi is running: config, rules and sidecar are read at startup; close it first" }

$iniSources=@{
  "Forced"=(Join-Path $fx "ProfessionGear.forced-test.ini");
  "Normal"=(Join-Path $repo "mod\ProfessionGear.ini");
  "NormalVerbose"=(Join-Path $fx "ProfessionGear.normal-verbose.ini");
  "AutoClassifyOff"=(Join-Path $fx "ProfessionGear.autoclassify-off.ini");
  "Disabled"=(Join-Path $fx "ProfessionGear.disabled.ini")
}
if($Mode -ne "Keep"){
  Copy-Item $iniSources[$Mode] (Join-Path $modDir "ProfessionGear.ini") -Force
  Write-Host ("ProfessionGear config: "+$Mode)
}
if($Rules -ne "Keep"){
  $src=if($Rules -eq "InGameTest"){ Join-Path $fx "ProfessionGear.ingame-test.rules" } else { Join-Path $repo "mod\ProfessionGear.rules" }
  Copy-Item $src (Join-Path $modDir "ProfessionGear.rules") -Force
  Write-Host ("ProfessionGear rules: "+$Rules)
}
$db=Join-Path $modDir "profession_gear_affixes.tsv"
$aside=Join-Path $modDir "profession_gear_affixes.tsv.set-aside"
if($Sidecar -eq "StartupTest" -or $Sidecar -eq "Empty"){
  if((Test-Path $db) -and -not (Test-Path $aside)){ Move-Item $db $aside }
  $src=if($Sidecar -eq "StartupTest"){ "profession_gear_affixes.startup-test.tsv" } else { "profession_gear_affixes.empty.tsv" }
  Copy-Item (Join-Path $fx $src) $db -Force
  Write-Host ("ProfessionGear sidecar: "+$Sidecar+" (previous sidecar set aside: "+$aside+")")
}
if($Sidecar -eq "Restore"){
  if(-not (Test-Path $aside)){ throw "nothing set aside: $aside" }
  Move-Item $aside $db -Force
  Write-Host "ProfessionGear sidecar restored"
}
Write-Host ("Mod folder: "+$modDir)
