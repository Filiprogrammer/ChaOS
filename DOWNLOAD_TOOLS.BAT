@echo off
cd %~d0%~p0

if exist tools goto downloaded

powershell -command "Invoke-WebRequest -OutFile \"%~d0%~p0chaostools.zip\" -Uri http://filip.mg6.at/chaostools.zip"
powershell -command "Expand-Archive -Force \"%~d0%~p0chaostools.zip\" \"%~d0%~p0tools\\""

:downloaded
