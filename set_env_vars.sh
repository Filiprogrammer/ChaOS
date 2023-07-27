#!/bin/sh

if [ -z $CHAOS_TOOLS_PATH_SET ]; then
    if [ -z $BASH_SOURCE ]; then
        cd "$(dirname "$(readlink -f "$0")")"
    else
        cd "$(dirname "$(readlink -f "$BASH_SOURCE")")"
    fi

    PWD=$(pwd)
    export PATH="$PWD/tools:$PWD/tools/i686-elf-tools-linux/bin:$PATH"
    export CHAOS_TOOLS_PATH_SET=1
    echo "Environment variables have been set"
else
    echo "Environment variables are already set"
fi
