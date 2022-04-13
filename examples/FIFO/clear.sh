#!/usr/bin/env bash

clear

path=$(
    cd "$(dirname "$0")"
    pwd -P
)

echo "path:" $path

rm -rf $path/build 2>/dev/null
