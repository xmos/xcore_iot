#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD toolchain"
    applications=(
        "test_hil_uart_tx_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "test_hil_uart_tx_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
 
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

    (cd ${path}; rm -rf build_ci_${board})
    (cd ${path}; mkdir -p build_ci_${board})
    (cd ${path}/build_ci_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application} -j)
done
