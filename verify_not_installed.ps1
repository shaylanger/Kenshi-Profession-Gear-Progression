$ErrorActionPreference = "Stop"
$known = @(
  "D:\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression",
  "C:\Program Files (x86)\Steam\steamapps\common\Kenshi\mods\ProfessionGearProgression"
)
$present=@()
foreach($p in $known){
  if(Test-Path $p){ $present += $p }
  else { Write-Host "SAFE    absent: $p" }
}
if($present.Count){
  Write-Error ("ProfessionGearProgression is already installed in "+$present.Count+" known game path(s).")
  exit 1
}
Write-Host "Game-directory safety check passed: ProfessionGearProgression not installed in known paths."
