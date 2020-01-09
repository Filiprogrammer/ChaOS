@echo off

cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

cd src

cecho {09}Assembling {#}{\t}==^> start.asm...
nasmw -O32 -f elf start.asm -o ../obj/start.o || goto ERROR
cecho {0a}done{#}{\n}

cecho {09}Compiling {#}{\t}==^> userlib.c...
i586-elf-gcc -std=c99 -c -Werror -Wall -O -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -ffunction-sections -fdata-sections -Iinclude userlib.c -o ../obj/userlib.o || goto ERROR
cecho {0a}done{#}{\n}

cd %~d0%~p0

setlocal EnableDelayedExpansion

FOR /R obj %%G IN (*.o) DO (
    set "zyx=!zyx! %%G"
)

i586-elf-ar cr userlib.lib %zyx%

endlocal

goto EOF

:ERROR
cd %~d0%~p0
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
