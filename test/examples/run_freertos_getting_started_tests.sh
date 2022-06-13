#!/bin/bash
set -e

BUILD_DIR="build"
APPLICATION="example_freertos_getting_started"

uname=`uname`
if [[ "$uname" == 'Linux' ]]; then
    TIMEOUT_EXE="timeout"
elif [[ "$uname" == 'Darwin' ]]; then
    TIMEOUT_EXE="gtimeout"
fi

DURATION="10s"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

($TIMEOUT_EXE $DURATION xrun --xscope $ADAPTER_ID $BUILD_DIR/$APPLICATION.xe 2>&1 | tee $APPLICATION.log)

# Search the log file for strings that indicate the app ran OK

# Expect exactly one match found
result_blinky_tile0=$(grep -c "Blinky task running from tile 0" $APPLICATION.log || true)

# Expect one or more matches found
result_hello_tile0=$(grep -c "Hello from tile 0" $APPLICATION.log || true)
result_hello_tile1=$(grep -c "Hello from tile 1" $APPLICATION.log || true)

if [ $result_hello_tile0 -le 1 ] || [ $result_hello_tile1 -le 1 ] || [ $result_blinky_tile0 -ne 1 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
