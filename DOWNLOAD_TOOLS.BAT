@echo off
cd %~d0%~p0

if exist tools\qemu-windows goto downloaded

powershell -command "Invoke-WebRequest -OutFile \"%~d0%~p0chaostools-windows.zip\" -Uri http://filip.mg6.at/chaostools-windows.zip"
powershell -command "Expand-Archive -Force \"%~d0%~p0chaostools-windows.zip\" \"%~d0%~p0tools\\""

:downloaded
