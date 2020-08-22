#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/*.obj 2>/dev/null
rm obj/object_files.lst 2>/dev/null
rm UefiLib.lib 2>/dev/null
