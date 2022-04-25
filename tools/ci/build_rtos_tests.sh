#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "name make_target BOARD toolchain"
applications=(
    "clock_control  test_rtos_driver_clock_control_test  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "hil            test_rtos_driver_hil                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "hil_add        test_rtos_driver_hil_add             XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "usb            test_rtos_driver_usb                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "wifi           test_rtos_driver_wifi                XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    name="${FIELDS[0]}"
    make_target="${FIELDS[1]}"
    board="${FIELDS[2]}"
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[3]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${make_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${make_target} -j)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR})
done
