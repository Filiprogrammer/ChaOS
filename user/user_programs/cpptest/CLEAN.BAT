@echo off
cd %~d0%~p0
del CPPTEST.ELF >nul 2>&1
del program.map >nul 2>&1
del program.o >nul 2>&1
