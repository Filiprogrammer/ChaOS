#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BasePrintLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -I../MdePkg/Include -I../MdePkg/Include/Ia32"

printf "\033[1;34mCompiling\033[1;97m\t==> PrintLib.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/PrintLib.obj src/PrintLib.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> PrintLibInternal.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/PrintLibInternal.obj src/PrintLibInternal.c || exit 1
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mCreating Archive\033[1;97m\t==> BasePrintLib.lib...\033[0m"
i686-elf-ar cr BasePrintLib.lib obj/PrintLib.obj obj/PrintLibInternal.obj || exit 1
printf "\033[1;32mdone\033[0m\n"
