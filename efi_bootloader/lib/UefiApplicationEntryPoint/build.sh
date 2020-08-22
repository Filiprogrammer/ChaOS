#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

printf "\033[1;34mCompiling\033[1;97m\t==> ApplicationEntryPoint.c...\033[0m"
i686-elf-gcc -g -fshort-wchar -fno-leading-underscore -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=UefiApplicationEntryPointStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -o obj/ApplicationEntryPoint.obj -I../MdePkg/Include -I../MdePkg/Include/Ia32 src/ApplicationEntryPoint.c || exit 1
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mCreating Archive\033[1;97m\t==> UefiApplicationEntryPoint.lib...\033[0m"
i686-elf-ar cr UefiApplicationEntryPoint.lib obj/ApplicationEntryPoint.obj || exit 1
printf "\033[1;32mdone\033[0m\n"
