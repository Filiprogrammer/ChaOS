#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"
rm libuser.a 2>/dev/null
rm obj\*.o 2>/dev/null
