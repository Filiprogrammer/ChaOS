@echo off
cd %~d0%~p0

cd BaseDebugPrintErrorLevelLib
call CLEAN.BAT
cd ..

cd BaseLib
call CLEAN.BAT
cd ..

cd BaseMemoryLib
call CLEAN.BAT
cd ..

cd BasePcdLibNull
call CLEAN.BAT
cd ..

cd BasePrintLib
call CLEAN.BAT
cd ..

cd UefiApplicationEntryPoint
call CLEAN.BAT
cd ..

cd UefiBootServicesTableLib
call CLEAN.BAT
cd ..

cd UefiDebugLibStdErr
call CLEAN.BAT
cd ..

cd UefiDevicePathLib
call CLEAN.BAT
cd ..

cd UefiLib
call CLEAN.BAT
cd ..

cd UefiMemoryAllocationLib
call CLEAN.BAT
cd ..

cd UefiRuntimeServicesTableLib
call CLEAN.BAT
cd ..
