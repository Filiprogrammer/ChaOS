#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh
mkdir obj 2>/dev/null

CFLAGS="-g -fshort-wchar -fno-builtin -fno-strict-aliasing -Wall -Werror -Wno-array-bounds -ffunction-sections -fdata-sections -include src/AutoGen.h -fno-common -DSTRING_ARRAY_NAME=BaseLibStrings -m32 -march=i586 -malign-double -fno-stack-protector -D EFI32 -fno-asynchronous-unwind-tables -Wno-address -fno-pic -fno-pie -Os -D DISABLE_NEW_DEPRECATED_INTERFACES -c -Isrc -Isrc/Ia32 -I../MdePkg/Include -I../MdePkg/Include/Ia32"

printf "\033[1;34mCompiling\033[1;97m\t==> DivS64x64Remainder.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/Ia32/DivS64x64Remainder.obj src/Ia32/DivS64x64Remainder.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> GccInline.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/Ia32/GccInline.obj src/Ia32/GccInline.c || exit 1
printf "\033[1;32mdone\033[0m\n"
printf "\033[1;34mCompiling\033[1;97m\t==> Non-existing.c...\033[0m"
i686-elf-gcc $CFLAGS -o obj/Ia32/Non-existing.obj src/Ia32/Non-existing.c || exit 1
printf "\033[1;32mdone\033[0m\n"

for srcfile in src/*.c; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mCompiling\033[1;97m\t==> $srcfilebase...\033[0m"
    i686-elf-gcc $CFLAGS -o obj/${srcfilebase%.c}.obj $srcfile || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

CFLAGS="-E -x assembler-with-cpp -include src/AutoGen.h -Isrc/Ia32 -Isrc -I../MdePkg/Include -I../MdePkg/Include/Ia32"
NASMFLAGS="-IIa32/ -I. -I../MdePkg/Include/ -I../MdePkg/Include/Ia32/ -f elf32"

for srcfile in src/Ia32/*.nasm; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mAssembling\033[1;97m\t==> $srcfilebase...\033[0m"
    i686-elf-gcc $CFLAGS $srcfile > obj/Ia32/${srcfilebase%.nasm}.i || exit 1
    Trim --trim-long --source-code -o obj/Ia32/${srcfilebase%.nasm}.iii obj/Ia32/${srcfilebase%.nasm}.i || exit 1
    nasm $NASMFLAGS -o obj/Ia32/${srcfilebase%.nasm}.obj obj/Ia32/${srcfilebase%.nasm}.iii || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

for objfile in $(find obj -name '*.obj'); do
    echo "\"$(pwd)/$objfile\"" >> obj/object_files.lst
done

printf "\033[1;34mCreating Archive\033[1;97m\t==> BaseLib.lib...\033[0m"
i686-elf-ar cr BaseLib.lib @obj/object_files.lst || exit 1
printf "\033[1;32mdone\033[0m\n"
