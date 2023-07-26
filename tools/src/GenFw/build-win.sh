#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"

printf "\033[1;32mBuilding Common...\033[0m\n"
cd Common
./build-win.sh || error
cd ..
printf "\033[1;32mCommon was built\033[0m\n\n"

printf "\033[1;32mBuilding GenFw...\033[0m\n"
cd GenFw
./build-win.sh || error
mv GenFw.exe ../../../
cd ..
printf "\033[1;32mGenFw was built\033[0m\n\n"
