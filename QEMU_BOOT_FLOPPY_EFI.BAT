@echo off
cd %~d0%~p0
tools\qemu-windows\qemu-system-i386.exe -audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound -bios tools/bios32.bin -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a
