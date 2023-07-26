#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

rm obj/*.o 2>/dev/null
