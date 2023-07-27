#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

usage() {
    echo "Usage: $0 <floppy|hd> [options]"
    echo "Options:"
    echo "  --debug                       Produce debugging information"
    echo "  --test                        Build tests"
    echo "  --efi                         Add the EFI bootloader to the image"
    echo "  --mbr                         Add a Master Boot Record to the image"
    echo "  --user-programs-all           Add all user programs to the image"
    echo "  --user-programs-none          Add no user program to the image"
    echo "  --add-user-program <program>  Add a <program> to the image"
    exit 1
}

IMAGE_TYPE=
DEBUG=0
TEST=0
EFI=0
MBR=0
BUILD_ARGS=

while [ $# -gt 0 ] ; do
    case "${1}" in
        (floppy) IMAGE_TYPE=Floppy ;;
        (hd) IMAGE_TYPE=HD ;;
        (--debug) DEBUG=1; BUILD_ARGS="$BUILD_ARGS --debug" ;;
        (--test) TEST=1; BUILD_ARGS="$BUILD_ARGS --test" ;;
        (--efi) EFI=1 ;;
        (--mbr) MBR=1 ;;
        (--user-programs-all) BUILD_ARGS="$BUILD_ARGS --user-programs-all" ;;
        (--user-programs-none) BUILD_ARGS="$BUILD_ARGS --user-programs-none" ;;
        (--add-user-program) BUILD_ARGS="$BUILD_ARGS --add-user-program $2" ; shift ;;
        *) usage ;;
    esac
    shift
done

if [ -z $IMAGE_TYPE ]; then usage; fi
if [ "$IMAGE_TYPE" = "Floppy" ] && [ $MBR = 1 ]; then echo "A Master Boot Record cannot be used on a floppy image"; exit 1; fi

cd "$(dirname "$(readlink -f "$0")")"
set -- $BUILD_ARGS
. "$(pwd)/build.sh" || error

if [ "$EFI" = "1" ]; then
    cd efi_bootloader
    printf "\033[1;32mBuilding EFI Bootloader...\033[0m\n"
    ./build.sh || error
    printf "\033[1;32mEFI Bootloader was built\033[0m\n"
    cd ..
fi

printf "\033[1;32mBuilding $IMAGE_TYPE Image...\033[0m\n"

if [ "$IMAGE_TYPE" = "HD" ]; then
    if [ "$MBR" = "1" ]; then
        mkdosfs -hd -mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin -m mbr_bootloader/boot_mbr.bin HDImage.bin || error
    else
        mkdosfs -hd -no-mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin HDImage.bin || error
    fi

    image_file=HDImage.bin
elif [ "$IMAGE_TYPE" = "Floppy" ]; then
    mkdosfs -fd -spt18 -heads2 -cyls80 -fats2 -fatz12 -spc1 -noverb -b stage1_bootloader/boot.bin FloppyImage.bin || error
    image_file=FloppyImage.bin
fi

imgtools -i $image_file -l ChaOS0.1 || error
imgtools -i $image_file stage2_bootloader/BOOT2.SYS || error
imgtools -i $image_file kernel/CHAOSKRN.SYS || error
imgtools -i $image_file res/STARTICO.BMP || error
imgtools -i $image_file res/WALLPAPR.BMP || error
imgtools -i $image_file res/MOUSEPTR.BMP || error

if [ "$EFI" = "1" ]; then
    imgtools -i $image_file -d EFI || error
    imgtools -i $image_file -d EFI/BOOT || error
    imgtools -i $image_file -f EFI/BOOT/BOOTIA32.EFI efi_bootloader/BOOTIA32.EFI || error
fi

for user_program_path in $user_programs_paths; do
    imgtools -i $image_file $user_program_path
done

if [ "$IMAGE_TYPE" = "HD" ]; then
    rm HDImage.vdi 2>/dev/null
    qemu-img convert -f raw -O vdi HDImage.bin HDImage.vdi || { printf "\033[1;31mFailed building VDI HD image\033[0m\n"; error; }
    printf '\161\115\052\323\313\174\144\110\246\204\311\327\307\244\045\365' | dd of=HDImage.vdi bs=1 seek=$((0x188)) conv=notrunc
fi

printf "\033[1;32m$IMAGE_TYPE Image was built\033[0m\n"
