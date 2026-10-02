@echo off
setlocal
set ROOT=%~dp0
call "%ROOT%run_tests.bat"
if errorlevel 1 exit /b 1
call "%ROOT%build_portable.bat"
if errorlevel 1 exit /b 1
call "%ROOT%verify_offline.bat"
if errorlevel 1 exit /b 1
call "%ROOT%package.bat"
if errorlevel 1 exit /b 1
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%verify_not_installed.ps1"
if errorlevel 1 exit /b 1
echo.
echo READY FOR CONTROLLED KENSHI INSTALL/TEST
exit /b 0
