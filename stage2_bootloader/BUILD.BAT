@echo off
cd %~d0%~p0
call CLEAN.BAT

cecho {09}Assembling{0F}{\t}==^> boot2.asm...{#}
nasm -f bin boot2.asm -o BOOT2.SYS || goto ERROR
cecho {0a}done{#}{\n}
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
