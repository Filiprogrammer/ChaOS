@echo off
cd %~d0%~p0
del kernel.map >nul 2>&1
del chaoskrn.sys >nul 2>&1
del chaoskrn.sys_debug >nul 2>&1
del obj\*.o >nul 2>&1
