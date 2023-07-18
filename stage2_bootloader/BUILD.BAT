@echo off
cd %~d0%~p0
call CLEAN.BAT

echo|set /p dummy="[94mAssembling[97m	==> boot2.asm...[0m"
nasm -f bin boot2.asm -o BOOT2.SYS || goto ERROR
echo [92mdone[0m
goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:EOF
