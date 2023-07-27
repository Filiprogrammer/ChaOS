#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

mkdir obj 2>/dev/null

for srcfile in src/*.c; do
    srcfilebase=$(basename "$srcfile")
    printf "\033[1;34mCompiling\033[1;97m\t==> $srcfilebase...\033[0m"
    gcc -c $srcfile -o obj/${srcfilebase%.c}.o || exit 1
    printf "\033[1;32mdone\033[0m\n"
done

gcc -s -o imgtools obj/*.o
mv imgtools ../../
