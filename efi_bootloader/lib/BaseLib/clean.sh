#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/*.obj 2>/dev/null
rm obj/Ia32/*.i 2>/dev/null
rm obj/Ia32/*.iii 2>/dev/null
rm obj/Ia32/*.obj 2>/dev/null
rm BaseLib.lib 2>/dev/null
rm obj/object_files.lst 2>/dev/null
