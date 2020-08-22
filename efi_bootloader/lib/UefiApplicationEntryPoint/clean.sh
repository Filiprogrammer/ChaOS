#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/ApplicationEntryPoint.obj 2>/dev/null
rm UefiApplicationEntryPoint.lib 2>/dev/null
