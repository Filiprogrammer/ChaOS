#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

mkdir obj 2>/dev/null

cd src

printf "\033[1;34mAssembling\033[1;97m\t==> start.asm...\033[0m"
nasm -O32 -f elf start.asm -o ../obj/start.o || error
printf "\033[1;32mdone\033[0m\n"

printf "\033[1;34mCompiling\033[1;97m\t==> userlib.c...\033[0m"
i686-elf-gcc -std=c99 -c -Werror -Wall -O -ffreestanding -fleading-underscore -nostdlib -nostdinc -fno-builtin -ffunction-sections -fdata-sections -Iinclude userlib.c -o ../obj/userlib.o || error
printf "\033[1;32mdone\033[0m\n"

cd ..

i686-elf-ar crs libuser.a $(ls obj/*.o | tr '\n' ' ')
