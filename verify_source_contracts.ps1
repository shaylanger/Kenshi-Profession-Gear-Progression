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

Write-Host "Source contract verification passed: 3 checks"
