#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/*.obj 2>/dev/null
rm BOOTIA32.EFI 2>/dev/null
rm efiboot.lib 2>/dev/null
rm efiboot.map 2>/dev/null
rm obj/object_files.lst 2>/dev/null
rm efiboot.dll 2>/dev/null
rm efiboot.debug 2>/dev/null
rm efiboot.txt 2>/dev/null
