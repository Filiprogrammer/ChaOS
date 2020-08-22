#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm kernel.map 2>/dev/null
rm CHAOSKRN.SYS 2>/dev/null
rm CHAOSKRN.SYS_debug 2>/dev/null
rm obj/*.o 2>/dev/null
rm obj-test/*.o 2>/dev/null
