@echo off
cd %~d0%~p0
call CLEAN.BAT
mingw32-make --makefile=Windows_makefile || goto ERROR
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
