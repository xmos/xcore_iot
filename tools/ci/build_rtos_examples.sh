#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD toolchain"
    applications=(
        "example_freertos_cifar10_extmem   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_cifar10_sram     XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_cifar10_swmem    XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_device_control   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_dispatcher       XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_explorer_board   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_getting_started  XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_iot              XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_l2_cache         XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "example_freertos_cifar10_extmem   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_cifar10_sram     XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_cifar10_swmem    XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_device_control   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_dispatcher       XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_explorer_board   XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_getting_started  XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_iot              XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
        "example_freertos_l2_cache         XCORE-AI-EXPLORER  tools/cmake_utils/xmos_xs3a_toolchain.cmake"
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
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[2]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board}; log_errors make ${application} -j)
done
