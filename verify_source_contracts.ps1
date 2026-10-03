$ErrorActionPreference="Stop"
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$p=Join-Path $root "src\ProfessionGearPlugin.cpp"
$text=Get-Content $p -Raw

function Require-Ordered([string]$scopeStart,[string]$first,[string]$second,[string]$label){
  $s=$text.IndexOf($scopeStart)
  if($s -lt 0){ throw "Missing scope for $label" }
  $a=$text.IndexOf($first,$s)
  $b=$text.IndexOf($second,$s)
  if($a -lt 0 -or $b -lt 0 -or $a -ge $b){ throw "Source contract failed: $label" }
  Write-Host "OK      $label"
}

Require-Ordered "void HookCraft" "if(g_craftOrig) g_craftOrig(b,item);" "if(item) EnsureRecord(item,crafter,true);" "craft final-quality ordering"

if($text -notmatch [regex]::Escape("if (!pack || !pack->isEquipped) return base;")){
  throw "Source contract failed: specialist backpack equip-only guard"
}
Write-Host "OK      specialist backpack equip-only guard"

if($text -notmatch [regex]::Escape("if(g_started) return;")){
  throw "Source contract failed: plugin double-start guard"
}
Write-Host "OK      plugin double-start guard"

if($text -notmatch [regex]::Escape('state->sdata[kPersistentIdField]=id;') -or
   $text -notmatch [regex]::Escape('state->activeValues[kPersistentIdField]=true;')){
  throw "Source contract failed: persistent item ID is not written into serialized GameData"
}
Write-Host "OK      persistent item ID serialized into GameData"

if($text -notmatch [regex]::Escape('BindPersistentItemId(item,id);')){
  throw "Source contract failed: persistent item ID is not restored from serialized GameData"
}
Write-Host "OK      persistent item ID restored from GameData"

if($text -notmatch [regex]::Escape('?serialiseInInventory@Item@@UEAAPEAVGameData@@PEAVGameDataContainer@@PEAV2@@Z') -or
   $text -notmatch [regex]::Escape('?loadFromSerialiseInInventory@Item@@UEAAXPEAVGameDataContainer@@PEAVGameData@@@Z')){
  throw "Source contract failed: item save/load hooks not installed"
}
Write-Host "OK      item save/load hooks installed"

Require-Ordered "void HookPlayerUpdate" "ProcessCharacter(*i);" "ProcessTraderShop(world,*i,false);" "trader shop storage scanned with the character scan"

if($text -notmatch [regex]::Escape('?buyItem@Inventory@@QEAAPEAVItem@@PEAV2@PEAVRootObject@@@Z') -or
   $text -notmatch [regex]::Escape('BindPersistentItemId(bought,shopId);')){
  throw "Source contract failed: purchase does not keep the shop item's persistent ID"
}
Write-Host "OK      purchase keeps the shop item's persistent ID"

if($text -notmatch [regex]::Escape('if(!itemId.empty() && ItemCurrentlyEligible(item,itemId)) equippedIds.push_back(itemId);')){
  throw "Source contract failed: stale/ineligible records still count toward equipped bonuses"
}
Write-Host "OK      stale/ineligible records give no equipped bonus"

Write-Host "Source contract verification passed: 9 checks"
