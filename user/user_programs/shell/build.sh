#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

make --makefile=makefile SHELL=sh || error
