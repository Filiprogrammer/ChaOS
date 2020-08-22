#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm boot.bin 2>/dev/null
