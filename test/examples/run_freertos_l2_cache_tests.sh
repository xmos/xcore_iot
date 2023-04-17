#!/bin/bash
set -e

XCORE_IOT_ROOT=`git rev-parse --show-toplevel`
source ${XCORE_IOT_ROOT}/tools/ci/helper_functions.sh

BUILD_DIR="${XCORE_IOT_ROOT}/dist"
APPLICATION="example_freertos_l2_cache"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

TIMEOUT_EXE=$(get_timeout)
DURATION="15s"

APP_XE=${BUILD_DIR}/${APPLICATION}.xe
APP_FLASH_CONTENTS=${BUILD_DIR}/${APPLICATION}_flash.bin
APP_LOG=${APPLICATION}.log

(xflash $ADAPTER_ID --force --quad-spi-clock 50MHz --write-all $APP_FLASH_CONTENTS --target XCORE-AI-EXPLORER)
($TIMEOUT_EXE $DURATION xrun $ADAPTER_ID --xscope $APP_XE 2>&1 | tee $APP_LOG)

# Search the log file for strings that indicate the app ran OK

# Expect one or more matches found
result=$(grep -c "Run examples" $APP_LOG || true)

if [ $result -le 1 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
