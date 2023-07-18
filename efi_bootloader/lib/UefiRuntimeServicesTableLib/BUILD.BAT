@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

echo|set /p dummy="[94mCompiling[97m	==> UefiRuntimeServicesTableLib.c...[0m"
i686-elf-gcc -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiRuntimeServicesTableLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/UefiRuntimeServicesTableLib.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/UefiRuntimeServicesTableLib.c || exit /B 1
echo [92mdone[0m

echo|set /p dummy="[94mCreating Archive[97m	==> UefiRuntimeServicesTableLib.lib...[0m"
i686-elf-ar cr UefiRuntimeServicesTableLib.lib obj/UefiRuntimeServicesTableLib.obj || exit /B 1
echo [92mdone[0m
