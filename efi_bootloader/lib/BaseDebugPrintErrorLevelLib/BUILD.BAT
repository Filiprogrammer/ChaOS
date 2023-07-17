@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

echo|set /p dummy="[94mCompiling[97m	==> BaseDebugPrintErrorLevelLib.c...[0m"
i686-elf-gcc -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseDebugPrintErrorLevelLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/BaseDebugPrintErrorLevelLib.obj -I../MdePkg -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/BaseDebugPrintErrorLevelLib.c || exit /B 1
echo [92mdone[0m

echo|set /p dummy="[94mCreating Archive[97m	==> BaseDebugPrintErrorLevelLib.lib...[0m"
i686-elf-ar cr BaseDebugPrintErrorLevelLib.lib obj/BaseDebugPrintErrorLevelLib.obj || exit /B 1
echo [92mdone[0m
