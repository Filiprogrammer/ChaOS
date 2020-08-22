@echo off

if "%CHAOS_TOOLS_PATH_SET%"=="TRUE" goto SKIP

set CHAOS_TOOLS_PATH_SET=TRUE
set PATH=%~d0%~p0tools\python27-windows;%~d0%~p0tools\i686-elf-tools-windows\bin;%~d0%~p0tools\python27-windows\Scripts;%~d0%~p0tools;%PATH%
set PYTHONPATH=%~d0%~p0tools\python27-windows\Lib
set PYTHONHOME=%~d0%~p0tools\python27-windows
echo Environment variables have been set
goto EOF

:SKIP
echo Environment variables are already set

:EOF
