#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

mkdir obj 2>/dev/null
CFLAGS="-c -I ../Include/ -I ../Common/src -I src -I ../MdePkg/Include/X64/ -I ../MdePkg/Include -fshort-wchar -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -nostdlib -O2"

for srcfile in src/*.c; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mCompiling\033[1;97m\t==> $srcfilebase...\033[0m"
    gcc $CFLAGS $srcfile -o obj/${srcfilebase%.c}.o || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

gcc -s -o GenFw obj/*.o ../Common/libCommon.a
