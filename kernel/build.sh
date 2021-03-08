#!/bin/sh

error() {
    printf "\033[1;31mError\033[0m\n"
    exit 1
}

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --debug   Produce debugging information"
    echo "  --test    Build tests"
    exit 1
}

cd "$(dirname "$(readlink -f "$0")")"
./clean.sh

DEBUG=0
TEST=0

while [ $# -gt 0 ] ; do
    case "${1}" in
        (--debug) DEBUG=1 ;;
        (--test) TEST=1 ;;
        *) usage ;;
    esac
    shift
done

version_string="ChaOS [Ver 0.2.$(date +'%y%m%d')] made by Filiprogrammer\n"
version_checksum=$(BSDChecksum "$version_string")

mkdir obj 2>/dev/null
mkdir obj-test 2>/dev/null

make DEBUG=$DEBUG TEST=$TEST VERSION_STRING="$version_string" VERSION_CHECKSUM=$version_checksum --makefile=makefile || error
