@echo off
cd %~d0%~p0
call SET_ENV_VARS.BAT
if not exist kernel\chaoskrn.sys_debug cecho {0C}WARNING! chaoskrn.sys_debug does not exist{#}{\n}
start /b cmd /c tools\qemu-windows\qemu-system-i386 -audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a -S -s
tools\gdb-multiarch -x kernel_debug.gdb
