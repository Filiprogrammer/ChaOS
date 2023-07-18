@echo off
cd %~d0%~p0
call CLEAN.BAT

set DEBUG=0
set TEST=0

:argLoop

if "%1" neq "" (
    if "%1"=="--debug" (
        set DEBUG=1
    ) else if "%1"=="--test" (
        set TEST=1
    ) else (
        goto USAGE
    )

    shift
    goto argLoop
)

set version_string=ChaOS [Ver 0.2.%date:~8,2%%date:~3,2%%date:~0,2%] made by Filiprogrammer\n

mkdir obj >nul 2>&1
mkdir obj-test >nul 2>&1

make DEBUG=%DEBUG% TEST=%TEST% VERSION_STRING="%version_string%" --makefile=makefile SHELL=cmd || goto ERROR

goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:USAGE
echo Usage: %0 [options]
echo Options:
echo   --debug   Produce debugging information
echo   --test    Build tests
EXIT /B 1

:EOF
