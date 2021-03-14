@echo off
cd %~d0%~p0
call CLEAN.BAT
make --makefile=makefile SHELL=cmd || goto ERROR
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
