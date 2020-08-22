#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiDevicePathLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Library/UefiDevicePathLib -I../MdePkg/Include -I../MdePkg/Include/Ia32"

printf "\033[1;34mCompiling\033[1;97m\t==> DevicePathFromText.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/DevicePathFromText.obj src/DevicePathFromText.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> DevicePathToText.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/DevicePathToText.obj src/DevicePathToText.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> DevicePathUtilities.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/DevicePathUtilities.obj src/DevicePathUtilities.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> UefiDevicePathLib.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/UefiDevicePathLib.obj src/UefiDevicePathLib.c || exit 1
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mCreating Archive\033[1;97m\t==> UefiDevicePathLib.lib...\033[0m"
i686-elf-ar cr UefiDevicePathLib.lib obj/DevicePathUtilities.obj obj/DevicePathToText.obj obj/DevicePathFromText.obj obj/UefiDevicePathLib.obj || exit 1
printf "\033[1;32mdone\033[0m\n"
