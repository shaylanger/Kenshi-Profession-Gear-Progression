$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$sdkLib = "C:\StobeBuild\sdk\KenshiLib.lib"
$dll = Join-Path $root "out\ProfessionGearProgression.dll"
$dumpbin = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio","C:\Program Files\Microsoft Visual Studio" -Recurse -Filter dumpbin.exe -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName
if (-not $dumpbin) { throw "dumpbin.exe not found" }
if (-not (Test-Path $sdkLib)) { throw "KenshiLib.lib not found: $sdkLib" }
if (-not (Test-Path $dll)) { throw "Built DLL not found: $dll" }

$libText = & $dumpbin /LINKERMEMBER:1 $sdkLib 2>&1 | Out-String
$required = @(
  "?update@PlayerInterface@@QEAAXXZ",
  "?getStat@CharStats@@QEBAMW4StatsEnumerated@@_N@Z",
  "?addFinishedCraftItem@CraftingBuilding@@QEAAXPEAVItem@@@Z",
  "?getTotalWeight@Inventory@@QEAAMXZ",
  "?_sectionAddItemCallback@Inventory@@UEAAXPEAVItem@@@Z",
  "?_sectionRemoveItemCallback@Inventory@@UEAAXPEAVItem@@@Z",
  "?_sectionUpdateItemCallback@Inventory@@UEAAXPEAVItem@@H@Z",
  "?serialiseInInventory@Item@@UEAAPEAVGameData@@PEAVGameDataContainer@@PEAV2@@Z",
  "?loadFromSerialiseInInventory@Item@@UEAAXPEAVGameDataContainer@@PEAVGameData@@@Z",
  "?isStackable@InventoryItemBase@@QEBAHPEAVInventorySection@@@Z",
  '?getSection@Inventory@@QEBAPEAVInventorySection@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z',
  "?isATrader@Character@@QEBA_NXZ",
  "?isPlayerCharacter@Character@@QEBA_NXZ",
  "?buyItem@Inventory@@QEAAPEAVItem@@PEAV2@PEAVRootObject@@@Z",
  '?getObjectsWithinSphere@GameWorld@@QEAAXAEAV?$lektor@PEAVRootObject@@@@AEBVVector3@Ogre@@MW4itemType@@HPEAVRootObject@@@Z',
  "?getTooltipData1@InventoryItemBase",
  "?getTooltipData1@Armour",
  "?getTooltipData1@ContainerItem",
  "?getTooltipData1@Crossbow",
  "?getTooltipData1@Sword"
)

$missing = @()
foreach ($symbol in $required) {
  if ($libText -notmatch [regex]::Escape($symbol)) {
    $missing += $symbol
    Write-Host "MISSING $symbol"
  } else {
    Write-Host "OK      $symbol"
  }
}

$exports = & $dumpbin /EXPORTS $dll 2>&1 | Out-String
$entry = "?startPlugin@@YAXXZ"
if ($exports -notmatch [regex]::Escape($entry)) {
  $missing += "DLL export $entry"
  Write-Host "MISSING DLL export $entry"
} else {
  Write-Host "OK      DLL export $entry"
}

if ($missing.Count -gt 0) {
  Write-Error ("Offline symbol verification failed: " + $missing.Count + " missing requirement(s)")
  exit 1
}
Write-Host ("Offline symbol verification passed: " + ($required.Count + 1) + " checks")
