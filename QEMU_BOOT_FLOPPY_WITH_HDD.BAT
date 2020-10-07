@echo off
cd %~d0%~p0
set /p hddpath="Path to the HDD Image: "
tools\qemu-windows\qemu-system-i386.exe -audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -drive format=raw,file=%hddpath%,index=0,if=ide -boot a
pause
