#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm BOOT2.SYS 2>/dev/null
rm boot2.map 2>/dev/null
