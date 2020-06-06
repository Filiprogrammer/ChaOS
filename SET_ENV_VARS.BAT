@echo off

if "%CHAOS_TOOLS_PATH_SET%"=="TRUE" goto SKIP

set CHAOS_TOOLS_PATH_SET=TRUE
set PATH=%PATH%;%~d0%~p0tools\crosstools-complete\bin;%~d0%~p0tools\Python27;%~d0%~p0tools\Python27\Scripts;%~d0%~p0tools
set PYTHONPATH=%~d0%~p0tools\Python27\Lib
set PYTHONHOME=%~d0%~p0tools\Python27
echo Environment variables have been set
goto EOF

:SKIP
echo Environment variables are already set

:EOF
