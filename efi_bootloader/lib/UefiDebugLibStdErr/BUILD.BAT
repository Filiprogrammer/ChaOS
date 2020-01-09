@echo off
cd %~d0%~p0
call CLEAN.BAT
mkdir obj >nul 2>&1

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiBootServicesTableLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32

cecho {09}Compiling {#}{\t}==^> DebugLib.c...
i586-elf-gcc %CFLAGS% -o obj/DebugLib.obj src/DebugLib.c || exit /B 1
cecho {0a}done{#}{\n}
cecho {09}Compiling {#}{\t}==^> DebugLibConstructor.c...
i586-elf-gcc %CFLAGS% -o obj/DebugLibConstructor.obj src/DebugLibConstructor.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> UefiDebugLibStdErr.lib...
i586-elf-ar cr UefiDebugLibStdErr.lib obj/DebugLib.obj obj/DebugLibConstructor.obj || exit /B 1
cecho {0a}done{#}{\n}
