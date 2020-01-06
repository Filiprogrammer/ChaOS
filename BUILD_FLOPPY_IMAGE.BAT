@echo off
cd %~d0%~p0
call BUILD.BAT %1

cecho {0A}Building Floppy Image...{\n0F}
mkdosfs -fd -spt18 -heads2 -cyls80 -fats2 -fatz12 -spc1 -noverb -b stage1_bootloader/boot.bin FloppyImage.bin || goto ERROR
imgtools -i FloppyImage.bin -l ChaOS0.1 || goto ERROR
imgtools -i FloppyImage.bin stage2_bootloader/BOOT2.SYS || goto ERROR
imgtools -i FloppyImage.bin kernel/CHAOSKRN.SYS || goto ERROR
imgtools -i FloppyImage.bin res/STARTICO.BMP || goto ERROR
imgtools -i FloppyImage.bin res/WALLPAPR.BMP || goto ERROR
imgtools -i FloppyImage.bin res/MOUSEPTR.BMP || goto ERROR
if "%user_programs_paths%"=="" goto imageBuilt
for %%a in ("%user_programs_paths: =" "%") do (
    if %%a NEQ "" imgtools -i FloppyImage.bin %%a
)
:imageBuilt
if not errorlevel 0 cecho {0C}Failed building floppy image{\n0F}&goto eof
cecho {0A}Floppy Image was built{\n0F}
goto eof

:ERROR
cecho {0C}Build failed{#}{\n}
cd %~d0%~p0
EXIT /B 1

:eof
