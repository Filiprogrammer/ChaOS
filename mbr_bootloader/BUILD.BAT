@echo off
cd %~d0%~p0
call CLEAN.BAT

echo|set /p dummy="[94mAssembling[97m	==> boot_mbr.asm...[0m"
nasm -f bin boot_mbr.asm -o boot_mbr.bin || goto ERROR
echo [92mdone[0m
goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:EOF
