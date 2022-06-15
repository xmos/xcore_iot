#!/bin/bash
set -e

# add dist_host to path.
#   This is used in CI for fatfs_mkimage
PATH=$PATH:./dist_host

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist
mkdir -p ${DIST_DIR}

# row format is: "name app_target run_fs_target BOARD toolchain"
ls dist_host/*
echo "***********"
ls *
echo "***********"
echo $PATH
applications=(
    "explorer_board   example_freertos_explorer_board   Yes XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "getting_started  example_freertos_getting_started  No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
)

# applications=(
#     "cifar10          example_freertos_cifar10          No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "device_control   example_freertos_device_control   No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "dispatcher       example_freertos_dispatcher       No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "explorer_board   example_freertos_explorer_board   Yes XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "getting_started  example_freertos_getting_started  No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "iot              example_freertos_iot              No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
#     "l2_cache         example_freertos_l2_cache         No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
# )

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    name="${FIELDS[0]}"
    app_target="${FIELDS[1]}"
    run_fs_target="${FIELDS[2]}"
    board="${FIELDS[3]}"
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[4]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${make_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board}; log_errors make ${app_target} -j)
    (cd ${path}/build_${board}; cp ${app_target}.xe ${DIST_DIR})
    if [ "$run_fs_target" = "Yes" ]; then
        (cd ${path}/build_${board}; log_errors make make_fs_${app_target} -j)
        (cd ${path}/build_${board}; cp ${app_target}_fat.fs ${DIST_DIR})
    fi
done
