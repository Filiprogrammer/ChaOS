@echo off
cd %~d0%~p0
call CLEAN.BAT

if not exist ..\stage2_bootloader\boot2.map echo [91mYou have to build the second stage bootloader first[0m&&goto ERROR
for /f "tokens=1" %%i in ('findstr /R /C:"[0-9a-fA-F][0-9a-fA-F]*  *OperatingSystemName" ..\stage2_bootloader\boot2.map') do set BPB_OFFSET=0x%%i

echo|set /p dummy="[94mAssembling[97m	==> boot.asm...[0m"
nasm -f bin -D BPB_OFFSET=%BPB_OFFSET% boot.asm -o boot.bin || goto ERROR
echo [92mdone[0m
goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:EOF
