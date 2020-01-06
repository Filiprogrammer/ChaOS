@echo off
cd %~d0%~p0
call BUILD.BAT %1

cecho {0A}Building HD Image...{\n0F}
mkdosfs -hd -no-mbr -fats2 -fatz12 -spc1 -z2 -noverb -b stage1_bootloader/boot.bin HDImage.bin || goto ERROR
imgtools -i HDImage.bin -l ChaOS0.1 || goto ERROR
imgtools -i HDImage.bin stage2_bootloader/BOOT2.SYS || goto ERROR
imgtools -i HDImage.bin kernel/CHAOSKRN.SYS || goto ERROR
imgtools -i HDImage.bin res/STARTICO.BMP || goto ERROR
imgtools -i HDImage.bin res/WALLPAPR.BMP || goto ERROR
imgtools -i HDImage.bin res/MOUSEPTR.BMP || goto ERROR
if "%user_programs_paths%"=="" goto imageBuilt
for %%a in ("%user_programs_paths: =" "%") do (
    if %%a NEQ "" imgtools -i HDImage.bin %%a
)
:imageBuilt
del HDImage.vdi >nul 2>&1
VBoxManage convertfromraw -format VDI HDImage.bin HDImage.vdi || goto ERROR
if not errorlevel 0 cecho {0C}Failed building HD image{\n0F}&goto eof
cecho {0A}HD Image was built{\n0F}
goto eof

:ERROR
cecho {0C}Build failed{#}{\n}
cd %~d0%~p0
EXIT /B 1

:eof
