@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseMemoryLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Isrc -I../MdePkg/Include -I../MdePkg/Include/Ia32

setlocal EnableDelayedExpansion

FOR %%G IN (src/*.c) DO (
    set "xyz=%%G"
    echo|set /p dummy="[94mCompiling[97m	==> %%G...[0m"
    i686-elf-gcc %CFLAGS% -o obj/!xyz:~,-2!.obj src/%%G || exit /B 1
    echo|set /p dummy="[92mdone[0m"
    echo.
)


FOR /R obj %%G IN (*.obj) DO (
    set "xyz="%%G^""
    ECHO !xyz:\=/!>>obj\object_files.lst
)

endlocal

echo|set /p dummy="[94mCreating Archive[97m	==> BaseMemoryLib.lib...[0m"
i686-elf-ar cr BaseMemoryLib.lib  @obj/object_files.lst || exit /B 1
echo [92mdone[0m
