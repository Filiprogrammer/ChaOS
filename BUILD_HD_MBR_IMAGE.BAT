@echo off
cd %~d0%~p0
call BUILD.BAT %1 || goto ERROR

cecho {0A}Building HD Image with MBR...{\n0F}
mkdosfs -hd -mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin -m mbr_bootloader/boot_mbr.bin HDImage.bin || goto ERROR
imgtools -i HDImage.bin -p1 -l ChaOS0.1 || goto ERROR
imgtools -i HDImage.bin -p1 stage2_bootloader/BOOT2.SYS || goto ERROR
imgtools -i HDImage.bin -p1 kernel/CHAOSKRN.SYS || goto ERROR
imgtools -i HDImage.bin -p1 res/STARTICO.BMP || goto ERROR
imgtools -i HDImage.bin -p1 res/WALLPAPR.BMP || goto ERROR
imgtools -i HDImage.bin -p1 res/MOUSEPTR.BMP || goto ERROR
if "%user_programs_paths%"=="" goto imageBuilt
for %%a in ("%user_programs_paths: =" "%") do (
    if %%a NEQ "" imgtools -i HDImage.bin -p1 %%a
)
:imageBuilt
del HDImage.vdi >nul 2>&1
VBoxManage convertfromraw --uuid d32a4d71-7ccb-4864-a684-c9d7c7a425f5 -format VDI HDImage.bin HDImage.vdi || cecho {0C}Failed building VDI HD image with MBR{\n0F}&&goto ERROR
cecho {0A}HD Image with MBR was built{\n0F}
goto eof

:ERROR
cecho {0C}Build failed{#}{\n}
cd %~d0%~p0
EXIT /B 1

:eof
