#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm obj/*.obj 2>/dev/null
rm BasePrintLib.lib 2>/dev/null
