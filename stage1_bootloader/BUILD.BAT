@echo off
cd %~d0%~p0
call CLEAN.BAT

if not exist ..\stage2_bootloader\boot2.map cecho {0C}You have to build the second stage bootloader first{#}{\n}&&goto ERROR
for /f "tokens=1" %%i in ('findstr /R /C:"[0-9a-fA-F][0-9a-fA-F]*  *OperatingSystemName" ..\stage2_bootloader\boot2.map') do set BPB_OFFSET=0x%%i

cecho {09}Assembling{0F}{\t}==^> boot.asm...{#}
nasm -f bin -D BPB_OFFSET=%BPB_OFFSET% boot.asm -o boot.bin || goto ERROR
cecho {0a}done{#}{\n}
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
