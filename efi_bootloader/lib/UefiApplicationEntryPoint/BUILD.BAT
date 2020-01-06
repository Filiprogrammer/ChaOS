@echo off
call CLEAN.BAT

cecho {09}Compiling {#}{\t}==^> ApplicationEntryPoint.c...
i586-elf-gcc -g -fshort-wchar -fno-leading-underscore -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiApplicationEntryPointStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/ApplicationEntryPoint.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/ApplicationEntryPoint.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> UefiApplicationEntryPoint.lib...
i586-elf-ar cr UefiApplicationEntryPoint.lib obj/ApplicationEntryPoint.obj || exit /B 1
cecho {0a}done{#}{\n}
