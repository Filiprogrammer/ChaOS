#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/UefiRuntimeServicesTableLib.obj 2>/dev/null
rm UefiRuntimeServicesTableLib.lib 2>/dev/null
