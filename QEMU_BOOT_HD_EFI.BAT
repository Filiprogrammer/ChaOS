@echo off
cd %~d0%~p0
tools\qemu\qemu-system-i386 -soundhw pcspk -bios bios32.bin -hda HDImage.bin -boot c
