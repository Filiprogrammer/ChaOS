@echo off
cd %~d0%~p0
del obj\ApplicationEntryPoint.obj >nul 2>&1
del UefiApplicationEntryPoint.lib >nul 2>&1
