@echo off
call CLEAN.BAT

set CFLAGS=-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiDevicePathLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Library/UefiDevicePathLib -I../MdePkg/Include -I../MdePkg/Include/Ia32

cecho {09}Compiling {#}{\t}==^> DevicePathFromText.c...
i586-elf-gcc %CFLAGS% -o obj/DevicePathFromText.obj src/DevicePathFromText.c || exit /B 1
cecho {0a}done{#}{\n}
cecho {09}Compiling {#}{\t}==^> DevicePathToText.c...
i586-elf-gcc %CFLAGS% -o obj/DevicePathToText.obj src/DevicePathToText.c || exit /B 1
cecho {0a}done{#}{\n}
cecho {09}Compiling {#}{\t}==^> DevicePathUtilities.c...
i586-elf-gcc %CFLAGS% -o obj/DevicePathUtilities.obj src/DevicePathUtilities.c || exit /B 1
cecho {0a}done{#}{\n}
cecho {09}Compiling {#}{\t}==^> UefiDevicePathLib.c...
i586-elf-gcc %CFLAGS% -o obj/UefiDevicePathLib.obj src/UefiDevicePathLib.c || exit /B 1
cecho {0a}done{#}{\n}

cecho {09}Creating Archive{#}{\t}==^> UefiDevicePathLib.lib...
i586-elf-ar cr UefiDevicePathLib.lib obj/DevicePathUtilities.obj obj/DevicePathToText.obj obj/DevicePathFromText.obj obj/UefiDevicePathLib.obj || exit /B 1
cecho {0a}done{#}{\n}
