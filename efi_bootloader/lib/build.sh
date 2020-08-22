#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"

printf "\033[1;32mBuilding BaseDebugPrintErrorLevelLib...\033[0m\n"
cd BaseDebugPrintErrorLevelLib
./build.sh || error
cd ..
printf "\033[1;32mBaseDebugPrintErrorLevelLib was built\033[0m\n\n"

printf "\033[1;32mBuilding BaseLib...\033[0m\n"
cd BaseLib
./build.sh || error
cd ..
printf "\033[1;32mBaseLib was built\033[0m\n\n"

printf "\033[1;32mBuilding BaseMemoryLib...\033[0m\n"
cd BaseMemoryLib
./build.sh || error
cd ..
printf "\033[1;32mBaseMemoryLib was built\033[0m\n\n"

printf "\033[1;32mBuilding BasePcdLibNull...\033[0m\n"
cd BasePcdLibNull
./build.sh || error
cd ..
printf "\033[1;32mBasePcdLibNull was built\033[0m\n\n"

printf "\033[1;32mBuilding BasePrintLib...\033[0m\n"
cd BasePrintLib
./build.sh || error
cd ..
printf "\033[1;32mBasePrintLib was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiApplicationEntryPoint...\033[0m\n"
cd UefiApplicationEntryPoint
./build.sh || error
cd ..
printf "\033[1;32mUefiApplicationEntryPoint was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiBootServicesTableLib...\033[0m\n"
cd UefiBootServicesTableLib
./build.sh || error
cd ..
printf "\033[1;32mUefiBootServicesTableLib was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiDebugLibStdErr...\033[0m\n"
cd UefiDebugLibStdErr
./build.sh || error
cd ..
printf "\033[1;32mUefiDebugLibStdErr was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiDevicePathLib...\033[0m\n"
cd UefiDevicePathLib
./build.sh || error
cd ..
printf "\033[1;32mUefiDevicePathLib was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiLib...\033[0m\n"
cd UefiLib
./build.sh || error
cd ..
printf "\033[1;32mUefiLib was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiMemoryAllocationLib...\033[0m\n"
cd UefiMemoryAllocationLib
./build.sh || error
cd ..
printf "\033[1;32mUefiMemoryAllocationLib was built\033[0m\n\n"

printf "\033[1;32mBuilding UefiRuntimeServicesTableLib...\033[0m\n"
cd UefiRuntimeServicesTableLib
./build.sh || error
cd ..
printf "\033[1;32mUefiRuntimeServicesTableLib was built\033[0m\n\n"
