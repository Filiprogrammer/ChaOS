@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1
mkdir obj\Ia32 >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Isrc -Isrc/Ia32 -I../MdePkg/Include -I../MdePkg/Include/Ia32

echo|set /p dummy="[94mCompiling[97m	==> DivS64x64Remainder.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/Ia32/DivS64x64Remainder.obj src/Ia32/DivS64x64Remainder.c || exit /B 1
echo [92mdone[0m
echo|set /p dummy="[94mCompiling[97m	==> GccInline.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/Ia32/GccInline.obj src/Ia32/GccInline.c || exit /B 1
echo [92mdone[0m
echo|set /p dummy="[94mCompiling[97m	==> Non-existing.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/Ia32/Non-existing.obj src/Ia32/Non-existing.c || exit /B 1
echo [92mdone[0m

setlocal EnableDelayedExpansion

FOR %%G IN (src/*.c) DO (
    set "xyz=%%G"
    echo|set /p dummy="[94mCompiling[97m	==> %%G...[0m"
    i686-elf-gcc %CFLAGS% -o obj/!xyz:~,-2!.obj src/%%G || exit /B 1
    echo|set /p dummy="[92mdone[0m"
    echo.
)

endlocal


set CFLAGS=-E -x assembler-with-cpp -include src/AutoGen.h -Isrc/Ia32 -Isrc -I../MdePkg/Include -I../MdePkg/Include/Ia32
set NASMFLAGS=-IIa32/ -I. -I../MdePkg/Include/ -I../MdePkg/Include/Ia32/ -f elf32

setlocal EnableDelayedExpansion

FOR %%G IN (src/Ia32/*.nasm) DO (
    set "xyz=%%G"
    set "yz=!xyz:~,-5!"
    echo|set /p dummy="[94mAssembling[97m	==> %%G...[0m"
    i686-elf-gcc %CFLAGS% src/Ia32/%%G > obj/Ia32/!yz!.i || exit /B 1
    call Trim --trim-long --source-code -o obj/Ia32/!yz!.iii obj/Ia32/!yz!.i || exit /B 1
    "nasm" %NASMFLAGS% -o obj/Ia32/!yz!.obj obj/Ia32/!yz!.iii || exit /B 1
    echo|set /p dummy="[92mdone[0m"
    echo.
)


FOR /R obj %%G IN (*.obj) DO (
    set "xyz="%%G^""
    ECHO !xyz:\=/!>>obj\object_files.lst
)

endlocal

echo|set /p dummy="[94mCreating Archive[97m	==> BaseLib.lib...[0m"
i686-elf-ar cr BaseLib.lib @obj/object_files.lst || exit /B 1
echo [92mdone[0m
