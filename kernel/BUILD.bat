@echo off
cd %~d0%~p0
call CLEAN.BAT

set DEBUG=0
set TEST=0

IF /i "%1"=="DEBUG" set DEBUG=1
IF /i "%2"=="DEBUG" set DEBUG=1
IF /i "%1"=="TEST" set TEST=1
IF /i "%2"=="TEST" set TEST=1

set version_string=ChaOS [Ver 0.2.%date:~8,2%%date:~3,2%%date:~0,2%] made by Filiprogrammer\n

for /f "tokens=*" %%a in ( 
'BSDChecksum.exe "%version_string%"' 
) do ( 
set version_checksum=%%a 
)

mkdir obj >nul 2>&1
mkdir obj-test >nul 2>&1

mingw32-make DEBUG=%DEBUG% TEST=%TEST% VERSION_STRING="%version_string%" VERSION_CHECKSUM=%version_checksum% --makefile=Windows_makefile || goto ERROR

goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
