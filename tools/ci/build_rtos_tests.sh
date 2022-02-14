#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD"
    applications=(
        "test/rtos_drivers/hil XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil_add XCORE-AI-EXPLORER"
        "test/rtos_drivers/usb XCORE-AI-EXPLORER"
        "test/rtos_drivers/wifi XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "test/rtos_drivers/hil XCORE-AI-EXPLORER"
    )
else
    echo "Argument $1 not a supported configuration!"
    exit
fi


# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    application="${FIELDS[0]}"
    board="${FIELDS[1]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DBOARD=${board}; log_errors make ${application} -j)
done
