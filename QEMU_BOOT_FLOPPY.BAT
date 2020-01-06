@echo off
cd %~d0%~p0
tools\qemu\qemu-system-i386 -soundhw pcspk -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a
