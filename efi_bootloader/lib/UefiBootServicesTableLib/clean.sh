#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/UefiBootServicesTableLib.obj 2>/dev/null
rm UefiBootServicesTableLib.lib 2>/dev/null
