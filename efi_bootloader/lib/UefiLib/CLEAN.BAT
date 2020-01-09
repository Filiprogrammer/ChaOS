@echo off
cd %~d0%~p0
del obj\*.obj >nul 2>&1
del obj\object_files.lst >nul 2>&1
del UefiLib.lib >nul 2>&1
