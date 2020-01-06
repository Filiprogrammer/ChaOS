@echo off

if "%CHAOS_TOOLS_PATH_SET%"=="TRUE" goto SKIP

set CHAOS_TOOLS_PATH_SET=TRUE
set PATH=%PATH%;%~d0%~p0tools\crosstools-complete\bin;%~d0%~p0tools
echo Environment variables have been set
goto EOF

:SKIP
echo Environment variables are already set

:EOF
