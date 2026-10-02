param(
  [switch]$Install,
  [string]$KenshiRoot = "D:\Steam\steamapps\common\Kenshi"
)
$ErrorActionPreference = "Stop"
if (-not $Install) {
  throw "Refusing to install. Re-run with -Install after Shay explicitly authorizes the live test."
}
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$src = Join-Path $root "out\package\ProfessionGearProgression"
if (-not (Test-Path (Join-Path $src "ProfessionGearProgression.dll"))) {
  throw "Package missing. Run build_portable.bat and package.bat first."
}
if (-not (Test-Path $KenshiRoot)) { throw "Kenshi root not found: $KenshiRoot" }
$mods = Join-Path $KenshiRoot "mods"
$dest = Join-Path $mods "ProfessionGearProgression"
if (-not (Test-Path $mods)) { throw "Kenshi mods directory not found: $mods" }

if (Test-Path $dest) {
  $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
  $backupRoot = Join-Path $mods "_ProfessionGearBackups"
  New-Item -ItemType Directory -Force -Path $backupRoot | Out-Null
  $backup = Join-Path $backupRoot ("ProfessionGearProgression-" + $stamp)
  Move-Item $dest $backup
  Write-Host "Backed up existing install to: $backup"
}

Copy-Item $src $dest -Recurse -Force
Write-Host "Installed test build to: $dest"
Write-Host "This script does not enable/change the Kenshi launcher load order."
