#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

mkdir obj 2>/dev/null

for srcfile in src/*.c; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mCompiling\033[1;97m\t==> $srcfilebase...\033[0m"
    x86_64-w64-mingw32-gcc -c $srcfile -o obj/${srcfilebase%.c}.o || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

x86_64-w64-mingw32-gcc -s -o imgtools.exe obj/*.o
mv imgtools.exe ../../
