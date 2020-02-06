@echo off
cd %~d0%~p0
call CLEAN.BAT

cd src
set version_string=ChaOS [Ver 0.2.%date:~8,2%%date:~3,2%%date:~0,2%] made by Filiprogrammer\n

for /f "tokens=*" %%a in ( 
'BSDChecksum.exe "%version_string%"' 
) do ( 
set version_checksum=%%a 
)

mkdir obj >nul 2>&1

IF /i "%1"=="DEBUG" (
    mingw32-make DEBUG=1 VERSION_STRING="%version_string%" VERSION_CHECKSUM=%version_checksum% --makefile=../Windows_makefile || goto ERROR
) ELSE (
    mingw32-make VERSION_STRING="%version_string%" VERSION_CHECKSUM=%version_checksum% --makefile=../Windows_makefile || goto ERROR
)

cd ..
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
