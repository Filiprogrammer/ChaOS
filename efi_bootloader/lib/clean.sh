#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

cd BaseDebugPrintErrorLevelLib
./clean.sh
cd ..

cd BaseLib
./clean.sh
cd ..

cd BaseMemoryLib
./clean.sh
cd ..

cd BasePcdLibNull
./clean.sh
cd ..

cd BasePrintLib
./clean.sh
cd ..

cd UefiApplicationEntryPoint
./clean.sh
cd ..

cd UefiBootServicesTableLib
./clean.sh
cd ..

cd UefiDebugLibStdErr
./clean.sh
cd ..

cd UefiDevicePathLib
./clean.sh
cd ..

cd UefiLib
./clean.sh
cd ..

cd UefiMemoryAllocationLib
./clean.sh
cd ..

cd UefiRuntimeServicesTableLib
./clean.sh
cd ..
