@echo off
call CLEAN.BAT

cecho {09}Compiling {#}{\t}==^> PcdLib.c...
i586-elf-gcc -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BasePcdLibNullStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/PcdLib.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/PcdLib.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> BasePcdLibNull.lib...
i586-elf-ar cr BasePcdLibNull.lib obj/PcdLib.obj || exit /B 1
cecho {0a}done{#}{\n}
