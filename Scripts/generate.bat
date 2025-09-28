
@echo off
REM PowerShell”Ågenerate.ps1‚ğŒÄ‚Ño‚µ‚Ü‚·
set SCRIPT_DIR=%~dp0
set PS_SCRIPT=%SCRIPT_DIR%generate.ps1
REM ˆø”‚ğ‚·‚×‚ÄPowerShell‚É“n‚·
powershell -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %*