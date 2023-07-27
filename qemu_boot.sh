#!/bin/sh

usage() {
    echo "Usage: $0 <floppy|hd> [options]"
    echo "Options:"
    echo "  --debug                       Run with debugger"
    echo "  --efi                         Enable EFI"
    echo "  --x64                         Emulate 64-bit CPU"
    echo "  --secondary-hd-image <image>  Add a secondary hard drive image"
    exit 1
}

IMAGE_TYPE=
DEBUG=0
EFI=0
SECONDARY_HD_IMAGE=
QEMU_CMD=qemu-system-i386
QEMU_ARGS="-audiodev pa,id=stdsound -machine pcspk-audiodev=stdsound"

while [ $# -gt 0 ] ; do
    case "${1}" in
        (floppy) IMAGE_TYPE=Floppy ;;
        (hd) IMAGE_TYPE=HD ;;
        (--debug) DEBUG=1 ;;
        (--efi) EFI=1 ;;
        (--x64) QEMU_CMD=qemu-system-x86_64 ;;
        (--secondary-hd-image) SECONDARY_HD_IMAGE="$2" ; shift ;;
        *) usage ;;
    esac
    shift
done

if [ -z $IMAGE_TYPE ]; then usage; fi
if [ "$EFI" = "1" ]; then QEMU_ARGS="$QEMU_ARGS -bios tools/bios32.bin"; fi

if [ ! -z "$SECONDARY_HD_IMAGE" ]; then
    QEMU_ARGS="$QEMU_ARGS -drive format=raw,file=\"$SECONDARY_HD_IMAGE\",index=1,if=ide"
fi

if [ "$IMAGE_TYPE" = "HD" ]; then
    QEMU_ARGS="$QEMU_ARGS -drive format=raw,file=HDImage.bin,index=0,if=ide -boot c"
elif [ "$IMAGE_TYPE" = "Floppy" ]; then
    QEMU_ARGS="$QEMU_ARGS -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a"
fi

if [ "$DEBUG" = "1" ]; then
    if [ ! -f kernel/CHAOSKRN.SYS_debug ]; then printf "\033[1;31mWARNING! CHAOSKRN.SYS_debug does not exist\033[0m\n"; fi

    (ERROR=$(eval $QEMU_CMD $QEMU_ARGS -S -s 3>&1 1>&2 2>&3 3>&- | tee /dev/tty)
    if echo "$ERROR" | grep -q "Could not initialize SDL\|gtk initialization failed"; then
        eval $QEMU_CMD -curses $QEMU_ARGS -S -s
    fi) &

    gdb -x kernel_debug.gdb
else
    ERROR=$(eval $QEMU_CMD $QEMU_ARGS 3>&1 1>&2 2>&3 3>&- | tee /dev/tty)
    if echo "$ERROR" | grep -q "Could not initialize SDL\|gtk initialization failed"; then
        eval $QEMU_CMD -curses $QEMU_ARGS
    fi
fi
