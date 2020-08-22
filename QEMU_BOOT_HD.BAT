@echo off
cd %~d0%~p0
tools\qemu-windows\qemu-system-i386 -audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound -drive format=raw,file=HDImage.bin,index=0,if=ide -boot c
