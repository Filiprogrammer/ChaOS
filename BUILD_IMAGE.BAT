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
    cecho {0A}Building EFI Bootloader...{#}{\n}
    call BUILD.BAT || goto ERROR
    cecho {0A}EFI Bootloader was built{#}{\n}
    cd ..
)

cecho {0A}Building %IMAGE_TYPE% Image...{#}{\n}

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
    VBoxManage convertfromraw --uuid d32a4d71-7ccb-4864-a684-c9d7c7a425f5 -format VDI HDImage.bin HDImage.vdi || cecho {0C}Failed building VDI HD image{#}{\n}&&goto ERROR
)
cecho {0A}%IMAGE_TYPE% Image was built{#}{\n}
goto eof

:ERROR
cecho {0C}Build failed{#}{\n}
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
