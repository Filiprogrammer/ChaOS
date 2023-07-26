#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

printf "\033[1;34mCompiling\033[1;97m\t==> mkdosfs.c...\033[0m"
gcc -s -o mkdosfs src/mkdosfs.c
mv mkdosfs ../../
printf "\033[1;32mdone\033[0m\n"
