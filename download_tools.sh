#!/bin/sh

cd "$(dirname "$(readlink -f "$0")")"

if [ -z "$(ls -A tools 2>/dev/null)" ]; then
    wget http://filip.mg6.at/chaostools.zip
    unzip chaostools.zip -d tools && rm chaostools.zip
    chmod -R 777 tools
fi
