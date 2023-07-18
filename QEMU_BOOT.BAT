@echo off
cd %~d0%~p0

set IMAGE_TYPE=
set DEBUG=0
set EFI=0
set SECONDARY_HD_IMAGE=
set QEMU_CMD=tools\qemu-windows\qemu-system-i386.exe
set QEMU_ARGS=-audiodev dsound,id=stdsound -machine pcspk-audiodev=stdsound

:argLoop

setlocal ENABLEDELAYEDEXPANSION

if "%1" neq "" (
    if "%1"=="floppy" (
        set IMAGE_TYPE=Floppy
    ) else if "%1"=="hd" (
        set IMAGE_TYPE=HD
    ) else if "%1"=="--debug" (
        set DEBUG=1
    ) else if "%1"=="--efi" (
        set EFI=1
    ) else if "%1"=="--x64" (
        set QEMU_CMD=tools\qemu-windows\qemu-system-x86_64.exe
    ) else if "%1"=="--secondary-hd-image" (
        set SECONDARY_HD_IMAGE=%2
        set SECONDARY_HD_IMAGE=!SECONDARY_HD_IMAGE:"=!
        shift
    ) else (
        goto USAGE
    )

    shift
    goto argLoop
)

endlocal

if "%IMAGE_TYPE%"=="" goto USAGE
if "%EFI%"=="1" set QEMU_ARGS=%QEMU_ARGS% -bios tools/bios32.bin
if "%SECONDARY_HD_IMAGE%" NEQ "" set QEMU_ARGS=%QEMU_ARGS% -drive format=raw,file="%SECONDARY_HD_IMAGE%",index=1,if=ide

if "%IMAGE_TYPE%"=="HD" (
    set QEMU_ARGS=%QEMU_ARGS% -drive format=raw,file=HDImage.bin,index=0,if=ide -boot c
) else if "%IMAGE_TYPE%"=="Floppy" (
    set QEMU_ARGS=%QEMU_ARGS% -drive format=raw,file=FloppyImage.bin,index=0,if=floppy -boot a
)

if "%DEBUG%"=="1" (
    call SET_ENV_VARS.BAT
    if not exist kernel\CHAOSKRN.SYS_debug echo [91mWARNING^^! CHAOSKRN.SYS_debug does not exist[0m
    start /b cmd /c %QEMU_CMD% %QEMU_ARGS% -S -s
    tools\gdb-multiarch.bat -x kernel_debug.gdb
) else (
    %QEMU_CMD% %QEMU_ARGS%
)

goto eof

:USAGE
echo Usage: %0 ^<floppy^|hd^> [options]
echo Options:
echo   --debug                       Run with debugger
echo   --efi                         Enable EFI
echo   --x64                         Emulate 64-bit CPU
echo   --secondary-hd-image ^<image^>  Add a secondary hard drive image
EXIT /B 1

:eof
