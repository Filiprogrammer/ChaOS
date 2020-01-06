@echo off
cd %~d0%~p0
call CLEAN.BAT

cecho {09}Assembling {#}{\t}==^> boot.asm...
nasmw -f bin boot.asm -o boot.bin || goto ERROR
cecho {0a}done{#}{\n}
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
