@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

cecho {09}Compiling {#}{\t}==^> UefiRuntimeServicesTableLib.c...
i586-elf-gcc -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiRuntimeServicesTableLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/UefiRuntimeServicesTableLib.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/UefiRuntimeServicesTableLib.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> UefiRuntimeServicesTableLib.lib...
i586-elf-ar cr UefiRuntimeServicesTableLib.lib obj/UefiRuntimeServicesTableLib.obj || exit /B 1
cecho {0a}done{#}{\n}
