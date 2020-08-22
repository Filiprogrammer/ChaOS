#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

select_dirs() {
    choices=$(dialog --stdout --no-items --ok-label "Finish" --checklist "Make your choice:" 0 0 0 $(find "$1" -mindepth 1 -maxdepth 1 -type d | sed "s#.*/##" | awk '{print $1, "on"}'))
}

cd "$(dirname "$(readlink -f "$0")")"
. "$(pwd)/set_env_vars.sh"

cd mbr_bootloader
printf "\033[1;32mBuilding Master Boot Record bootloader...\033[0m\n"
./build.sh || error
printf "\033[1;32mMaster Boot Record bootloader was built\033[0m\n\n"

cd ../stage2_bootloader
printf "\033[1;32mBuilding second stage bootloader...\033[0m\n"
./build.sh || error
printf "\033[1;32mSecond stage bootloader was built\033[0m\n\n"

cd ../stage1_bootloader
printf "\033[1;32mBuilding first stage bootloader...\033[0m\n"
./build.sh || error
printf "\033[1;32mFirst stage bootloader was built\033[0m\n\n"

cd ..

# User Library
cd user/user_tools
printf "\033[1;32mBuilding user library...\033[0m\n"
./build.sh || error
printf "\033[1;32mUser library was built\033[0m\n\n"
cd ..

# User Programs
cd user_programs
select_dirs "$(pwd)"
user_programs_paths=""

for program_name in $choices; do
    printf "\033[1;32mBuilding program ${program_name}...\033[0m\n"
    cd $program_name
    ./build.sh || { printf "\033[1;31mFailed building program \"${program_name}\"\033[0m\n"; error; }
    printf "\033[1;32mProgram \"${program_name}\" was built\033[0m\n\n"
    user_programs_paths="$user_programs_paths $(find * -maxdepth 0 -not -type d -iname "*.elf" | sed "s/^/user\/user_programs\/${program_name}\//")"
    cd ..
done

printf "\033[1;32mAll selected programs were built\033[0m\n\n"

# Kernel
cd ../../kernel
printf "\033[1;32mBuilding kernel...\033[0m\n"
./build.sh "$1" "$2" || { printf "\033[1;31mFailed building kernel\033[0m\n"; error; }
printf "\033[1;32mKernel was built\033[0m\n"

cd ..
