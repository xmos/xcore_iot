#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`
source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

BUILD_DIR="${XCORE_SDK_ROOT}/dist"
APPLICATION="example_freertos_tracealyzer"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

TIMEOUT_EXE=$(get_timeout)
DURATION="10s"

APP_XE=${BUILD_DIR}/${APPLICATION}.xe
APP_TRACE=${BUILD_DIR}/${APPLICATION}_trace
APP_LOG=${APPLICATION}.log

($TIMEOUT_EXE $DURATION xrun $ADAPTER_ID --xscope-file $APP_TRACE $APP_XE 2>&1 | tee $APP_LOG)

# Search the log file for strings that indicate the app ran OK

# Expect exactly one match found
result_main_task=$(grep -c "Entered main process" $APP_LOG || true)
result_subtask0=$(grep -c "Entered subprocess task (0)" $APP_LOG || true)
result_subtask1=$(grep -c "Entered subprocess task (1)" $APP_LOG || true)
result_subtask2=$(grep -c "Entered subprocess task (2)" $APP_LOG || true)
result_subtask3=$(grep -c "Entered subprocess task (3)" $APP_LOG || true)
result_subtask4=$(grep -c "Entered subprocess task (4)" $APP_LOG || true)
result_subtask5=$(grep -c "Entered subprocess task (5)" $APP_LOG || true)
result_subtask6=$(grep -c "Entered subprocess task (6)" $APP_LOG || true)
result_subtask7=$(grep -c "Entered subprocess task (7)" $APP_LOG || true)
result_gpio_task=$(grep -c "Entered gpio task" $APP_LOG || true)

# Expect one or more matches found
result_hello_tile0=$(grep -c "Hello from tile 0" $APP_LOG || true)
result_hello_tile1=$(grep -c "Hello from tile 1" $APP_LOG || true)

# Define a set of regex components to find the VCD line containing the PSF
# header. This header descibes the tracealyzer version/configuration and
# serves as a basic verification step that the FreeRTOS example has been
# configured to run tracealyzer.
PSF_HEADER_LEN="l32"
PSF_BOM="00465350"
PSF_FORMAT_VER="0a00"
PSF_PLTFM_ID="a11a"
PSF_OPTIONS="[0-9a-fA-F]\{8\}"
PSF_NUM_CORES="[0-9a-fA-F]\{8\}"
PSF_ISR_TAIL_CHAIN_THRES="[0-9a-fA-F]\{8\}"
PSF_PLTFM_CFG="4672656552544f53" # "FreeRTOS"
PSF_PLTFM_CFG_VER="[0-9a-fA-F]\{8\}"
PSF_PROBE="0"

PSF_HEADER_REGEX="^$PSF_HEADER_LEN $PSF_BOM$PSF_FORMAT_VER\
$PSF_PLTFM_ID$PSF_OPTIONS$PSF_NUM_CORES$PSF_ISR_TAIL_CHAIN_THRES\
$PSF_PLTFM_CFG$PSF_PLTFM_CFG_VER $PSF_PROBE"

# Check for the PSF header near the beginning of the VCD file.
# Only one occurance of this entry should be detected.
result_tracealyzer=$(head -n 10 $APP_TRACE.vcd | grep -c "$PSF_HEADER_REGEX")

if [ $result_hello_tile0 -le 1 ] || [ $result_hello_tile1 -le 1 ] ||
   [ $result_main_task -ne 1 ] ||
   [ $result_subtask0 -ne 1 ] || [ $result_subtask1 -ne 1 ] || [ $result_subtask2 -ne 1 ] ||
   [ $result_subtask3 -ne 1 ] || [ $result_subtask4 -ne 1 ] || [ $result_subtask5 -ne 1 ] ||
   [ $result_subtask6 -ne 1 ] || [ $result_subtask7 -ne 1 ] || [ $result_gpio_task -ne 1 ] ||
   [ $result_tracealyzer -ne 1 ]; then
    echo "FAIL"
    exit 1
fi

echo "PASS"
