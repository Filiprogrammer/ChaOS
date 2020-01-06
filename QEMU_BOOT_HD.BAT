@echo off
cd %~d0%~p0
tools\qemu\qemu-system-i386 -soundhw pcspk -hda HDImage.bin -boot c
