#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

[ -e ../stage2_bootloader/boot2.map ] || { printf "\033[1;31mYou have to build the second stage bootloader first\033[0m\n"; error; }
BPB_OFFSET=0x$(grep -o '[0-9a-fA-F]\+[ ]\+OperatingSystemName' ../stage2_bootloader/boot2.map | awk '{print $1}')

printf "\033[1;34mAssembling\033[1;97m\t==> boot.asm...\033[0m"
nasm -f bin -D BPB_OFFSET=$BPB_OFFSET boot.asm -o boot.bin || error
printf "\033[1;32mdone\033[0m\n"
