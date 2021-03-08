#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

select_dirs() {
    choices=$(dialog --stdout --no-items --ok-label "Finish" --checklist "Make your choice:" 0 0 0 $(find "$1" -mindepth 1 -maxdepth 1 -type d | sed "s#.*/##" | awk '{print $1, "on"}'))
}

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --debug                       Produce debugging information"
    echo "  --test                        Build tests"
    echo "  --user-programs-all           Build all user programs"
    echo "  --user-programs-none          Build no user program"
    echo "  --add-user-program <program>  Build a <program>"
    exit 1
}

DEBUG=0
TEST=0
BUILD_ARGS=
USER_PROGRAMS_ALL=0
USER_PROGRAMS_NONE=0
choices=

while [ $# -gt 0 ] ; do
    case "${1}" in
        (--debug) DEBUG=1; BUILD_ARGS="$BUILD_ARGS --debug" ;;
        (--test) TEST=1; BUILD_ARGS="$BUILD_ARGS --test" ;;
        (--user-programs-all) USER_PROGRAMS_ALL=1 ;;
        (--user-programs-none) USER_PROGRAMS_NONE=1 ;;
        (--add-user-program) choices="$choices $2" ; shift ;;
        *) usage ;;
    esac
    shift
done

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

if [ $USER_PROGRAMS_ALL = 1 ]; then
    choices=$(find "$(pwd)" -mindepth 1 -maxdepth 1 -type d | sed "s#.*/##")
elif [ $USER_PROGRAMS_NONE = 1 ]; then
    choices=" "
elif [ -z "$choices" ]; then
    select_dirs "$(pwd)"
fi

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
./build.sh $BUILD_ARGS || { printf "\033[1;31mFailed building kernel\033[0m\n"; error; }
printf "\033[1;32mKernel was built\033[0m\n"

cd ..
