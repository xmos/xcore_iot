#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`
source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

BUILD_DIR="${XCORE_SDK_ROOT}/dist"
APPLICATION="example_freertos_explorer_board"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

TIMEOUT_EXE=$(get_timeout)
DURATION="10s"

APP_XE=${BUILD_DIR}/${APPLICATION}.xe
APP_FS=${BUILD_DIR}/${APPLICATION}_fat.fs
APP_LOG=${APPLICATION}.log

(xflash $ADAPTER_ID --quad-spi-clock 50MHz --factory $APP_XE --boot-partition-size 0x100000 --data $APP_FS)
($TIMEOUT_EXE $DURATION xrun $ADAPTER_ID --xscope $APP_XE 2>&1 | tee $APP_LOG)

# Search the log file for strings that indicate the app ran OK

# Expect one or more matches found
result_hello_world=$(grep -c "Hello World!" $APP_LOG || true)

if [ $result_hello_world -le 1 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
