@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiBootServicesTableLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32

echo|set /p dummy="[94mCompiling[97m	==> DebugLib.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/DebugLib.obj src/DebugLib.c || exit /B 1
echo [92mdone[0m
echo|set /p dummy="[94mCompiling[97m	==> DebugLibConstructor.c...[0m"
i686-elf-gcc %CFLAGS% -o obj/DebugLibConstructor.obj src/DebugLibConstructor.c || exit /B 1
echo [92mdone[0m

echo|set /p dummy="[94mCreating Archive[97m	==> UefiDebugLibStdErr.lib...[0m"
i686-elf-ar cr UefiDebugLibStdErr.lib obj/DebugLib.obj obj/DebugLibConstructor.obj || exit /B 1
echo [92mdone[0m
