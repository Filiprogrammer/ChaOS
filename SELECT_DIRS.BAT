@echo off

setlocal ENABLEDELAYEDEXPANSION

set numberOfOptions=0

for /D %%f in (*.*) do (
	set options[!numberOfOptions!]=%%f
	set m[!numberOfOptions!]= 
	set /a numberOfOptions=!numberOfOptions!+1
)

set selected=0
set /a highestOption=%numberOfOptions%-1

:menu
cls
echo Make your choice:
echo.
FOR /L %%i IN (0,1,%highestOption%) DO (
	if "%selected%"=="%%i" (
		echo ^> [!m[%%i]!] !options[%%i]!
	) else echo   [!m[%%i]!] !options[%%i]!
)

echo.
echo Use WS to navigate up and down
echo Press Q to select an option
echo Press E when finished
choice /N /C:WSQE
if errorlevel 255 goto menu
if errorlevel 4 goto finish
if errorlevel 3 goto wahl
if errorlevel 2 goto runter
if errorlevel 1 goto hoch
if errorlevel 0 goto menu

:hoch
set /a "selected-=1"
if %selected% LSS 0 set selected=0
goto menu

:runter
set /a "selected+=1"
if %selected% GTR %highestOption% set selected=%highestOption%
goto menu

:wahl
if "!m[%selected%]!"==" " (
	set m[%selected%]=X
	goto menu
)
if "!m[%selected%]!"=="X" set m[%selected%]= 
goto menu

:finish
set result=
FOR /L %%i IN (0,1,%highestOption%) DO (
	if "!m[%%i]!"=="X" set result=!result! !options[%%i]!
)

endlocal & set ret=%result%

set "%~1=%ret%"
