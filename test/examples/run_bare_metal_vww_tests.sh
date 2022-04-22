#!/bin/bash
set -e

BUILD_DIR="build"
APPLICATION="example_bare_metal_vww"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

(xrun --xscope $ADAPTER_ID $BUILD_DIR/$APPLICATION.xe 2>&1 | tee $APPLICATION.log)

# Search the log file for "DONE!" to indicate a passed test
result=$(grep -c DONE! $APPLICATION.log || true)

if [ $result -ne 1 ]; then
    # Expect exactly one match found
    echo "FAIL"
    exit 1
fi

echo "PASS"

