@echo off
cd %~d0%~p0

set IMAGE_TYPE=
set DEBUG=0
set TEST=0
set EFI=0
set MBR=0
set BUILD_ARGS=

:argLoop

if "%1" neq "" (
    if "%1"=="floppy" (
        set IMAGE_TYPE=Floppy
    ) else if "%1"=="hd" (
        set IMAGE_TYPE=HD
    ) else if "%1"=="--debug" (
        set DEBUG=1
        set BUILD_ARGS=%BUILD_ARGS% --debug
    ) else if "%1"=="--test" (
        set TEST=1
        set BUILD_ARGS=%BUILD_ARGS% --test
    ) else if "%1"=="--efi" (
        set EFI=1
    ) else if "%1"=="--mbr" (
        set MBR=1
    ) else if "%1"=="--user-programs-all" (
        set BUILD_ARGS=%BUILD_ARGS% --user-programs-all
    ) else if "%1"=="--user-programs-none" (
        set BUILD_ARGS=%BUILD_ARGS% --user-programs-none
    ) else if "%1"=="--add-user-program" (
        set BUILD_ARGS=%BUILD_ARGS% --add-user-program %2
        shift
    ) else (
        goto USAGE
    )

    shift
    goto argLoop
)

if "%IMAGE_TYPE%"=="" goto USAGE
if "%IMAGE_TYPE%"=="Floppy" if "%MBR%"=="1" echo A Master Boot Record cannot be used on a floppy image && exit /B 1

call BUILD.BAT %BUILD_ARGS% || goto ERROR

if "%EFI%"=="1" (
    cd efi_bootloader
    echo [92mBuilding EFI Bootloader...[0m
    call BUILD.BAT || goto ERROR
    echo [92mEFI Bootloader was built[0m
    cd ..
)

echo [92mBuilding %IMAGE_TYPE% Image...[0m

if "%IMAGE_TYPE%"=="HD" (
    if "%MBR%"=="1" (
        mkdosfs -hd -mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin -m mbr_bootloader/boot_mbr.bin HDImage.bin || goto ERROR
    ) else (
        mkdosfs -hd -no-mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin HDImage.bin || goto ERROR
    )
    
    set image_file=HDImage.bin
) else if "%IMAGE_TYPE%"=="Floppy" (
    mkdosfs -fd -spt18 -heads2 -cyls80 -fats2 -fatz12 -spc1 -noverb -b stage1_bootloader/boot.bin FloppyImage.bin || goto ERROR
    set image_file=FloppyImage.bin
)

imgtools -i %image_file% -l ChaOS0.1 || goto ERROR
imgtools -i %image_file% stage2_bootloader/BOOT2.SYS || goto ERROR
imgtools -i %image_file% kernel/CHAOSKRN.SYS || goto ERROR
imgtools -i %image_file% res/STARTICO.BMP || goto ERROR
imgtools -i %image_file% res/WALLPAPR.BMP || goto ERROR
imgtools -i %image_file% res/MOUSEPTR.BMP || goto ERROR

if "%EFI%"=="1" (
    imgtools -i %image_file% -d EFI || goto ERROR
    imgtools -i %image_file% -d EFI/BOOT || goto ERROR
    imgtools -i %image_file% -f EFI/BOOT/BOOTIA32.EFI efi_bootloader/BOOTIA32.EFI || goto ERROR
)

if "%user_programs_paths%"=="" goto imageBuilt
for %%a in ("%user_programs_paths: =" "%") do (
    if %%a NEQ "" imgtools -i %image_file% %%a
)
:imageBuilt
if "%IMAGE_TYPE%"=="HD" (
    del HDImage.vdi >nul 2>&1
    qemu-img convert -f raw -O vdi HDImage.bin HDImage.vdi || echo [91mFailed building VDI HD image[0m&&goto ERROR
    python3 -c "f = open('HDImage.vdi', 'r+b'); f.seek(0x188); f.write(b'\x71\x4d\x2a\xd3\xcb\x7c\x64\x48\xa6\x84\xc9\xd7\xc7\xa4\x25\xf5'); f.close();"
)
echo [92m%IMAGE_TYPE% Image was built[0m
goto eof

:ERROR
echo [91mBuild failed[0m
cd %~d0%~p0
EXIT /B 1

:USAGE
echo Usage: %0 ^<floppy^|hd^> [options]
echo Options:
echo   --debug                       Produce debugging information
echo   --test                        Build tests
echo   --efi                         Add the EFI bootloader to the image
echo   --mbr                         Add a Master Boot Record to the image
echo   --user-programs-all           Add all user programs to the image
echo   --user-programs-none          Add no user program to the image
echo   --add-user-program ^<program^>  Add a ^<program^> to the image
EXIT /B 1

:eof
