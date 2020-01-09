@echo off
cd %~d0%~p0
del obj\*.obj >nul 2>&1
del UefiDebugLibStdErr.lib >nul 2>&1
