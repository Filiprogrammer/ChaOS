#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

DEBUG=0
TEST=0

[ ! -z $1 ] && if [ $1 = "DEBUG" ]; then DEBUG=1; fi
[ ! -z $1 ] && if [ $1 = "TEST" ]; then TEST=1; fi
[ ! -z $2 ] && if [ $2 = "DEBUG" ]; then DEBUG=1; fi
[ ! -z $2 ] && if [ $2 = "TEST" ]; then TEST=1; fi

version_string="ChaOS [Ver 0.2.$(date +'%y%m%d')] made by Filiprogrammer\n"
version_checksum=$(BSDChecksum "$version_string")

mkdir obj 2>/dev/null
mkdir obj-test 2>/dev/null

make DEBUG=$DEBUG TEST=$TEST VERSION_STRING="$version_string" VERSION_CHECKSUM=$version_checksum --makefile=makefile || error
