#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")/tools/qemu-linux"
export LD_LIBRARY_PATH="$(dirname "$(readlink -f "$0")")/lib"

(ERROR=$(./qemu-system-i386 -audiodev pa,id=stdsound -machine pcspk-audiodev=stdsound -bios ../bios32.bin -drive format=raw,file=../../HDImage.bin,index=0,if=ide -boot c -S -s 3>&1 1>&2 2>&3 3>&- | tee /dev/tty)
if echo "$ERROR" | grep -q "Could not initialize SDL\|gtk initialization failed"; then
    ./qemu-system-i386 -curses -audiodev pa,id=stdsound -machine pcspk-audiodev=stdsound -bios ../bios32.bin -drive format=raw,file=../../HDImage.bin,index=0,if=ide -boot c -S -s
fi) &

cd ../..
tools/gdb-multiarch.sh -x kernel_debug.gdb
