@echo off
cd %~d0%~p0
del kernel.map >nul 2>&1
del CHAOSKRN.SYS >nul 2>&1
del CHAOSKRN.SYS_debug >nul 2>&1
del obj\*.o >nul 2>&1
del obj-test\*.o >nul 2>&1
