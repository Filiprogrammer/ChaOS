#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm boot_mbr.bin 2>/dev/null
