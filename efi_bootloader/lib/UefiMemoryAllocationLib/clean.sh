#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/MemoryAllocationLib.obj 2>/dev/null
rm UefiMemoryAllocationLib.lib 2>/dev/null
