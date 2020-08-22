#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiBootServicesTableLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32"

printf "\033[1;34mCompiling\033[1;97m\t==> DebugLib.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/DebugLib.obj src/DebugLib.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> DebugLibConstructor.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/DebugLibConstructor.obj src/DebugLibConstructor.c || exit 1
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mCreating Archive\033[1;97m\t==> UefiDebugLibStdErr.lib...\033[0m"
i686-elf-ar cr UefiDebugLibStdErr.lib obj/DebugLib.obj obj/DebugLibConstructor.obj || exit 1
printf "\033[1;32mdone\033[0m\n"
