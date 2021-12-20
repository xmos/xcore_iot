#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configuraitons
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "path/to/application BOARD"
    applications=(
        "examples/freertos/cifar10 XCORE-AI-EXPLORER"
        "examples/freertos/cifar10 OSPREY-BOARD"
        "examples/freertos/explorer_board XCORE-AI-EXPLORER"
        "examples/freertos/iot_aws XCORE-AI-EXPLORER"
        "examples/freertos/person_detection XCORE-AI-EXPLORER"
        "examples/freertos/device_control XCORE200-MIC-ARRAY"
        "examples/freertos/getting_started XCORE-AI-EXPLORER"
        "examples/freertos/dispatcher XCORE-AI-EXPLORER"
        "examples/freertos/l2_cache XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "examples/freertos/getting_started XCORE-AI-EXPLORER"
        "examples/freertos/explorer_board XCORE-AI-EXPLORER"
        "examples/freertos/cifar10 XCORE-AI-EXPLORER"
        "examples/freertos/device_control XCORE200-MIC-ARRAY"
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
    path="${XCORE_SDK_ROOT}/${application}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; log_errors make distclean)
    (cd ${path}; log_errors make -j BOARD=${board})
done
