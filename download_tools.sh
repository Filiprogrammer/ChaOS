#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

if [ -z "$(ls -A tools/qemu-linux 2>/dev/null)" ]; then
    wget http://filip.mg6.at/chaostools-linux.zip
    unzip -o chaostools-linux.zip -d tools && rm chaostools-linux.zip
    chmod -R 777 tools
fi
