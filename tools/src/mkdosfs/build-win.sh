#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

printf "\033[1;34mCompiling\033[1;97m\t==> mkdosfs.c...\033[0m"
x86_64-w64-mingw32-gcc -s -o mkdosfs.exe src/mkdosfs.c
mv mkdosfs.exe ../../
printf "\033[1;32mdone\033[0m\n"
