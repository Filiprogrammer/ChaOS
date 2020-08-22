#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
. "$(pwd)/build.sh" "$1" "$2" || error

printf "\033[1;32mBuilding Floppy Image...\033[0m\n"
mkdosfs -fd -spt18 -heads2 -cyls80 -fats2 -fatz12 -spc1 -noverb -b stage1_bootloader/boot.bin FloppyImage.bin || error
imgtools -i FloppyImage.bin -l ChaOS0.1 || error
imgtools -i FloppyImage.bin stage2_bootloader/BOOT2.SYS || error
imgtools -i FloppyImage.bin kernel/CHAOSKRN.SYS || error
imgtools -i FloppyImage.bin res/STARTICO.BMP || error
imgtools -i FloppyImage.bin res/WALLPAPR.BMP || error
imgtools -i FloppyImage.bin res/MOUSEPTR.BMP || error

for user_program_path in $user_programs_paths; do
    imgtools -i FloppyImage.bin $user_program_path
done

printf "\033[1;32mFloppy Image was built\033[0m\n"
