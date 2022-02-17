#!/bin/bash
# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing_rpt
mkdir testing_rpt
TIMEOUT_S=60

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "path make_target BOARD"
    applications=(
        "test/rtos_drivers/clock_control test_rtos_driver_clock_control_test XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil test_rtos_driver_hil XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil_add test_rtos_driver_hil_add XCORE-AI-EXPLORER"
        "test/rtos_drivers/usb test_rtos_driver_usb XCORE-AI-EXPLORER"
        "test/rtos_drivers/wifi test_rtos_driver_wifi XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "test/rtos_drivers/clock_control test_rtos_driver_clock_control_test XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil test_rtos_driver_hil XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil_add test_rtos_driver_hil_add XCORE-AI-EXPLORER"
        "test/rtos_drivers/usb test_rtos_driver_usb XCORE-AI-EXPLORER"
        "test/rtos_drivers/wifi test_rtos_driver_wifi XCORE-AI-EXPLORER"
    )
else
    echo "Argument $1 not a supported configuration!"
    exit
fi

# run tests
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    app_path="${FIELDS[0]}"
    application="${FIELDS[1]}"
    board="${FIELDS[2]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Running' ${application} 'for' ${board}
    echo '******************************************************'
    echo 'Not Implemented'
    # if [ "$UNAME" == "Linux" ] ; then
    #     (cd ${path}/build_ci_${board}/${app_path}; timeout ${TIMEOUT_S}s xrun --xscope ${application}.xe 2>&1 | tee -a ${application})
    # elif [ "$UNAME" == "Darwin" ] ; then
    #     (cd ${path}/build_ci_${board}/${app_path}; gtimeout ${TIMEOUT_S}s xrun --xscope ${application}.xe 2>&1 | tee -a ${application})
    # fi
done
