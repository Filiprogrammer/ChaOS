@echo off
cd %~d0%~p0
set /p hddpath="Path to the HDD Image: "
tools\qemu\qemu-system-i386 -soundhw pcspk -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -hda %hddpath% -boot a
pause
