
@echo off
REM PowerShell��generate.ps1���Ăяo���܂�
set SCRIPT_DIR=%~dp0
set PS_SCRIPT=%SCRIPT_DIR%generate.ps1
REM ���������ׂ�PowerShell�ɓn��
powershell -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %*