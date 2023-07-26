#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

rm obj/*.o 2>/dev/null
rm GenFw 2>/dev/null
rm GenFw.exe 2>/dev/null
