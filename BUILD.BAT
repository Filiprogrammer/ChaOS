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
cecho {0A}Building Master Boot Record bootloader...{#}{\n}
call BUILD.BAT || goto ERROR
cecho {0A}Master Boot Record bootloader was built{#}{\n}
echo.

cd ..\stage2_bootloader
cecho {0A}Building second stage bootloader...{#}{\n}
call BUILD.BAT || goto ERROR
cecho {0A}Second stage bootloader was built{#}{\n\n}

cd ..\stage1_bootloader
cecho {0A}Building first stage bootloader...{#}{\n}
call BUILD.BAT || goto ERROR
cecho {0A}First stage bootloader was built{#}{\n\n}

cd ..

::User Library
cd user\user_tools
cecho {0A}Building user library...{#}{\n}
call BUILD.bat || goto ERROR
cecho {0A}User library was built{#}{\n\n}
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
    cecho {0A}Building program "%%a"...{#}{\n}
    cd %%a
    call BUILD.BAT || cecho {0C}Failed building program "%%a"{#}{\n}&&endlocal&&goto ERROR
    cecho {0A}Program "%%a" was built{#}{\n\n}
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
cecho {0A}All selected programs were built{#}{\n\n}
set errorlevel=0

::Kernel
cd ..\..\kernel
cecho {0A}Building kernel...{#}{\n}
call BUILD.BAT %BUILD_ARGS% || cecho {0C}Failed building kernel{#}{\n}&&goto ERROR
cecho {0A}Kernel was built{#}{\n}

cd ..

goto eof

:ERROR
cecho {0C}Build failed{#}{\n}
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
