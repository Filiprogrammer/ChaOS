#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm THRDTEST.ELF 2>/dev/null
rm program.map 2>/dev/null
rm program.o 2>/dev/null
