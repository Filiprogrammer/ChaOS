@echo off
cd %~d0%~p0
call SET_ENV_VARS.BAT
if not exist kernel\chaoskrn.sys_debug cecho {0C}WARNING! chaoskrn.sys_debug does not exist{#}{\n}
start /b cmd /c tools\qemu\qemu-system-i386 -soundhw pcspk -hda HDImage.bin -boot c -S -s
tools\gdb -x kernel_debug.gdb
