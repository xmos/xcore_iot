#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`
source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

BUILD_DIR="${XCORE_SDK_ROOT}/dist"
APPLICATION="example_freertos_dispatcher"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

TIMEOUT_EXE=$(get_timeout)
DURATION="10s"

APP_XE=${BUILD_DIR}/${APPLICATION}.xe
APP_LOG=${APPLICATION}.log

($TIMEOUT_EXE $DURATION xrun $ADAPTER_ID --xscope $APP_XE 2>&1 | tee $APP_LOG)

# Search the log file for strings that indicate the app ran OK

# Expect exactly one match found
result=$(grep -c "output matrix verified" $APP_LOG || true)

if [ $result -ne 1 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
