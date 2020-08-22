#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/PcdLib.obj 2>/dev/null
rm BasePcdLibNull.lib 2>/dev/null
