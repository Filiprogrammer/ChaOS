@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseMemoryLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Isrc -I../MdePkg/Include -I../MdePkg/Include/Ia32

setlocal EnableDelayedExpansion

FOR %%G IN (src/*.c) DO (
    set "xyz=%%G"
    cecho {09}Compiling {0F}{\t}==^> %%G...{#}
    i686-elf-gcc %CFLAGS% -o obj/!xyz:~,-2!.obj src/%%G || exit /B 1
    cecho {0a}done{#}{\n}
)


FOR /R obj %%G IN (*.obj) DO (
    set "xyz="%%G^""
    ECHO !xyz:\=/!>>obj\object_files.lst
)

endlocal

cecho {09}Creating Archive{0F}{\t}==^> BaseMemoryLib.lib...{#}
i686-elf-ar cr BaseMemoryLib.lib  @obj/object_files.lst || exit /B 1
cecho {0a}done{#}{\n}
