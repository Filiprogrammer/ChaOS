@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BasePrintLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32

echo|set /p dummy="[94mCompiling[97m	==> PrintLib.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/PrintLib.obj src/PrintLib.c || exit /B 1
echo [92mdone[0m
echo|set /p dummy="[94mCompiling[97m	==> PrintLibInternal.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/PrintLibInternal.obj src/PrintLibInternal.c || exit /B 1
echo [92mdone[0m

echo|set /p dummy="[94mCreating Archive[97m	==> BasePrintLib.lib...[0m"
i686-elf-ar cr BasePrintLib.lib obj/PrintLib.obj obj/PrintLibInternal.obj || exit /B 1
echo [92mdone[0m

exit /B
