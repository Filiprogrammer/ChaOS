@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BasePrintLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32

cecho {09}Compiling {#}{\t}==^> PrintLib.c...
i586-elf-gcc %CFLAGS% -o obj/PrintLib.obj src/PrintLib.c || exit /B 1
cecho {0a}done{#}{\n}
cecho {09}Compiling {#}{\t}==^> PrintLibInternal.c...
i586-elf-gcc %CFLAGS% -o obj/PrintLibInternal.obj src/PrintLibInternal.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> BasePrintLib.lib...
i586-elf-ar cr BasePrintLib.lib obj/PrintLib.obj obj/PrintLibInternal.obj || exit /B 1
cecho {0a}done{#}{\n}

exit /B
