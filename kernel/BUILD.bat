@echo off
cd %~d0%~p0
call CLEAN.BAT

cd src
echo #ifndef VERSION_H>include\version.h
echo #define VERSION_H>>include\version.h
set version_string=ChaOS [Ver 0.2.%date:~8,2%%date:~3,2%%date:~0,2%] made by Filiprogrammer\n
echo #define __VERSION_STRING "%version_string%">>include\version.h

for /f "tokens=*" %%a in ( 
'BSDChecksum.exe "%version_string%"' 
) do ( 
set version_checksum=%%a 
)

echo #define __VERSION_CHECKSUM %version_checksum%>>include\version.h
echo #endif>>include\version.h

IF /i "%1"=="DEBUG" (
    mingw32-make DEBUG=1 --makefile=../Windows_makefile || goto ERROR
) ELSE (
    mingw32-make --makefile=../Windows_makefile || goto ERROR
)

cd ..
goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
