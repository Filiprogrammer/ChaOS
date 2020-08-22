#!/bin/sh

error() {
    printf "\033[1;31mBuild failed\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
. "$(pwd)/build.sh" "$1" "$2" || error

printf "\033[1;32mBuilding HD Image with MBR...\033[0m\n"
mkdosfs -hd -mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin -m mbr_bootloader/boot_mbr.bin HDImage.bin || error
imgtools -i HDImage.bin -l ChaOS0.1 || error
imgtools -i HDImage.bin stage2_bootloader/BOOT2.SYS || error
imgtools -i HDImage.bin kernel/CHAOSKRN.SYS || error
imgtools -i HDImage.bin res/STARTICO.BMP || error
imgtools -i HDImage.bin res/WALLPAPR.BMP || error
imgtools -i HDImage.bin res/MOUSEPTR.BMP || error

for user_program_path in $user_programs_paths; do
    imgtools -i HDImage.bin $user_program_path
done

rm HDImage.vdi 2>/dev/null
VBoxManage convertfromraw --uuid d32a4d71-7ccb-4864-a684-c9d7c7a425f5 -format VDI HDImage.bin HDImage.vdi || { printf "\033[1;31mFailed building VDI HD image with MBR\033[0m\n"; error; }
printf "\033[1;32mHD Image with MBR was built\033[0m\n"
