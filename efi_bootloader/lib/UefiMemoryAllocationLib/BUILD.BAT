@echo off
call CLEAN.BAT

cecho {09}Compiling {#}{\t}==^> MemoryAllocationLib.c...
i586-elf-gcc -g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiMemoryAllocationLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/MemoryAllocationLib.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/MemoryAllocationLib.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> UefiMemoryAllocationLib.lib...
i586-elf-ar cr UefiMemoryAllocationLib.lib obj/MemoryAllocationLib.obj || exit /B 1
cecho {0a}done{#}{\n}

exit /B
