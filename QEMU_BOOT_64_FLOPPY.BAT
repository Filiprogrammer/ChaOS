@echo off
cd %~d0%~p0
tools\qemu-windows\qemu-system-x86_64.exe -audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a
