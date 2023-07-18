@echo off
cd %~d0%~p0

echo [92mBuilding BaseDebugPrintErrorLevelLib...[0m
cd BaseDebugPrintErrorLevelLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mBaseDebugPrintErrorLevelLib was built[0m
echo.

echo [92mBuilding BaseLib...[0m
cd BaseLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mBaseLib was built[0m
echo.

echo [92mBuilding BaseMemoryLib...[0m
cd BaseMemoryLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mBaseMemoryLib was built[0m
echo.

echo [92mBuilding BasePcdLibNull...[0m
cd BasePcdLibNull
call BUILD.BAT || goto ERROR
cd ..
echo [92mBasePcdLibNull was built[0m
echo.

echo [92mBuilding BasePrintLib...[0m
cd BasePrintLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mBasePrintLib was built[0m
echo.

echo [92mBuilding UefiApplicationEntryPoint...[0m
cd UefiApplicationEntryPoint
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiApplicationEntryPoint was built[0m
echo.

echo [92mBuilding UefiBootServicesTableLib...[0m
cd UefiBootServicesTableLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiBootServicesTableLib was built[0m
echo.

echo [92mBuilding UefiDebugLibStdErr...[0m
cd UefiDebugLibStdErr
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiDebugLibStdErr was built[0m
echo.

echo [92mBuilding UefiDevicePathLib...[0m
cd UefiDevicePathLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiDevicePathLib was built[0m
echo.

echo [92mBuilding UefiLib...[0m
cd UefiLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiLib was built[0m
echo.

echo [92mBuilding UefiMemoryAllocationLib...[0m
cd UefiMemoryAllocationLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiMemoryAllocationLib was built[0m
echo.

echo [92mBuilding UefiRuntimeServicesTableLib...[0m
cd UefiRuntimeServicesTableLib
call BUILD.BAT || goto ERROR
cd ..
echo [92mUefiRuntimeServicesTableLib was built[0m
echo.

goto EOF

:ERROR
echo [91mError[0m
EXIT /B 1

:EOF
