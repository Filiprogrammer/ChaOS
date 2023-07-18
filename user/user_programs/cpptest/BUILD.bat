@echo off
cd %~d0%~p0
call CLEAN.BAT
make --makefile=makefile SHELL=cmd || goto ERROR
goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:EOF
