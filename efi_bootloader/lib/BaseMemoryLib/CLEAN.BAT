@echo off
cd %~d0%~p0
del obj\*.obj >nul 2>&1
del obj\object_files.lst >nul 2>&1
del BaseMemoryLib.lib >nul 2>&1
