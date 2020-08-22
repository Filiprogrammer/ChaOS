#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseMemoryLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Isrc -I../MdePkg/Include -I../MdePkg/Include/Ia32"

for srcfile in src/*.c; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mCompiling\033[1;97m\t==> $srcfilebase...\033[0m"
    i686-elf-gcc $CFLAGS -o obj/${srcfilebase%.c}.obj $srcfile || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

for objfile in $(find obj -name '*.obj'); do
    echo "\"$(pwd)/$objfile\"" >> obj/object_files.lst
done

printf "\033[1;34mCreating Archive\033[1;97m\t==> BaseMemoryLib.lib...\033[0m"
i686-elf-ar cr BaseMemoryLib.lib @obj/object_files.lst || exit 1
printf "\033[1;32mdone\033[0m\n"
