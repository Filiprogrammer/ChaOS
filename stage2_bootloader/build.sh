#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

printf "\033[1;34mAssembling\033[1;97m\t==> boot2.asm...\033[0m"
nasm -f bin boot2.asm -o BOOT2.SYS || error
printf "\033[1;32mdone\033[0m\n"
