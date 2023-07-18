@echo off

set DEBUG=0
set TEST=0
set BUILD_ARGS=
set USER_PROGRAMS_ALL=0
set USER_PROGRAMS_NONE=0
set result=

:argLoop

if "%1" neq "" (
    if "%1"=="--debug" (
        set DEBUG=1
        set BUILD_ARGS=%BUILD_ARGS% --debug
    ) else if "%1"=="--test" (
        set TEST=1
        set BUILD_ARGS=%BUILD_ARGS% --test
    ) else if "%1"=="--user-programs-all" (
        set USER_PROGRAMS_ALL=1
    ) else if "%1"=="--user-programs-none" (
        set USER_PROGRAMS_NONE=1
    ) else if "%1"=="--add-user-program" (
        set result=%result% %2
        shift
    ) else (
        goto USAGE
    )

    shift
    goto argLoop
)

cd %~d0%~p0
call SET_ENV_VARS.BAT

cd mbr_bootloader
echo [92mBuilding Master Boot Record bootloader...[0m
call BUILD.BAT || goto ERROR
echo [92mMaster Boot Record bootloader was built[0m
echo.

cd ..\stage2_bootloader
echo [92mBuilding second stage bootloader...[0m
call BUILD.BAT || goto ERROR
echo [92mSecond stage bootloader was built[0m
echo.

cd ..\stage1_bootloader
echo [92mBuilding first stage bootloader...[0m
call BUILD.BAT || goto ERROR
echo [92mFirst stage bootloader was built[0m
echo.

cd ..

::User Library
cd user\user_tools
echo [92mBuilding user library...[0m
call BUILD.bat || goto ERROR
echo [92mUser library was built[0m
echo.
cd ..

::User Programs
cd user_programs

setlocal ENABLEDELAYEDEXPANSION

if "%USER_PROGRAMS_ALL%"=="1" (
    for /D %%f in (*.*) do set result=!result! %%f
) else if "%USER_PROGRAMS_NONE%"=="1" (
    set result=
) else if "%result%"=="" (
    call ..\..\SELECT_DIRS.BAT result
)

endlocal & set result=%result%

set i=1
set user_programs_paths=
setlocal ENABLEDELAYEDEXPANSION
:user_programs_loop
set errorlevel=1
FOR /F "tokens=%i%" %%a IN ("%result%") DO (
    set errorlevel=0
    echo [92mBuilding program "%%a"...[0m
    cd %%a
    call BUILD.BAT || echo [91mFailed building program "%%a"[0m&&endlocal&&goto ERROR
    echo [92mProgram "%%a" was built[0m
    echo.
    FOR /F %%b IN ('dir *.ELF /B /A:A') DO (
        set "usr_programs_paths=!usr_programs_paths! user/user_programs/%%a/%%b"
    )
    cd ..
)
if %errorlevel%==1 goto user_programs_loop_end
set /a i=%i%+1
goto user_programs_loop

:user_programs_loop_end
endlocal & set user_programs_paths=%usr_programs_paths%
echo [92mAll selected programs were built[0m
echo.
set errorlevel=0

::Kernel
cd ..\..\kernel
echo [92mBuilding kernel...[0m
call BUILD.BAT %BUILD_ARGS% || echo [91mFailed building kernel[0m&&goto ERROR
echo [92mKernel was built[0m

cd ..

goto eof

:ERROR
echo [91mBuild failed[0m
cd %~d0%~p0
EXIT /B 1

:USAGE
echo Usage: %0 [options]
echo Options:
echo   --debug                       Produce debugging information
echo   --test                        Build tests
echo   --user-programs-all           Build all user programs
echo   --user-programs-none          Build no user program
echo   --add-user-program ^<program^>  Build a ^<program^>
EXIT /B 1

:eof
