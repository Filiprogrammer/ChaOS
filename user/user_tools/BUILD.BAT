@echo off

cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

cd src

echo|set /p dummy="[94mAssembling[97m	==> start.asm...[0m"
nasm -O32 -f elf start.asm -o ../obj/start.o || goto ERROR
echo [92mdone[0m

echo|set /p dummy="[94mCompiling[97m	==> userlib.c...[0m"
i686-elf-gcc -std=c99 -c -Werror -Wall -O -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -ffunction-sections -fdata-sections -Iinclude userlib.c -o ../obj/userlib.o || goto ERROR
echo [92mdone[0m

cd %~d0%~p0

setlocal EnableDelayedExpansion

FOR /R obj %%G IN (*.o) DO (
    set "zyx=!zyx! ^"%%G^""
)

i686-elf-ar crs libuser.a %zyx%

endlocal

goto EOF

:ERROR
cd %~d0%~p0
echo [91mError[0m
EXIT /B 1

:EOF
