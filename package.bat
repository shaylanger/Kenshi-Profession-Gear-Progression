@echo off
setlocal
set ROOT=%~dp0
set DEST=%ROOT%out\package\ProfessionGearProgression
if not exist "%ROOT%out\ProfessionGearProgression.dll" (
  echo ERROR: build the DLL first with build_portable.bat
  exit /b 1
)
if exist "%DEST%" rmdir /s /q "%DEST%"
mkdir "%DEST%"
copy /y "%ROOT%out\ProfessionGearProgression.dll" "%DEST%\ProfessionGearProgression.dll" >nul
copy /y "%ROOT%mod\RE_Kenshi.json" "%DEST%\RE_Kenshi.json" >nul
copy /y "%ROOT%mod\ProfessionGearProgression.mod" "%DEST%\ProfessionGearProgression.mod" >nul
copy /y "%ROOT%mod\mod.info" "%DEST%\mod.info" >nul
copy /y "%ROOT%mod\ProfessionGear.ini" "%DEST%\ProfessionGear.ini" >nul
copy /y "%ROOT%mod\ProfessionGear.rules" "%DEST%\ProfessionGear.rules" >nul
copy /y "%ROOT%FIRST_LIVE_TEST.md" "%DEST%\FIRST_LIVE_TEST.md" >nul
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%verify_package.ps1"
if errorlevel 1 exit /b 1
echo PACKAGE OK: %DEST%
