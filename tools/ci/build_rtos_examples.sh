#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist
mkdir -p ${DIST_DIR}

# row format is: "name make_target BOARD toolchain"
applications=(
    "cifar10          example_freertos_cifar10          XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "device_control   example_freertos_device_control   XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "dispatcher       example_freertos_dispatcher       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "explorer_board   example_freertos_explorer_board   XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "getting_started  example_freertos_getting_started  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "iot              example_freertos_iot              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "l2_cache         example_freertos_l2_cache         XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
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
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board}; log_errors make ${make_target} -j)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR})
done
