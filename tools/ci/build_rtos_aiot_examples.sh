#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`
source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist
DIST_HOST_DIR=${XCORE_SDK_ROOT}/dist_host
mkdir -p ${DIST_DIR}

if [ -d "${DIST_HOST_DIR}" ]; then
    # add DIST_HOST_DIR to path.
    #   This is used in CI for fatfs_mkimage
    PATH="${DIST_HOST_DIR}":$PATH
    find ${DIST_HOST_DIR} -type f -exec chmod a+x {} +
fi

# row format is: "name app_target run_fs_target run_swmem_target board toolchain"
applications=(
    "example_freertos_iot              No  No  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    app_target="${FIELDS[0]}"
    run_fs_target="${FIELDS[1]}"
    run_swmem_target="${FIELDS[2]}"
    board="${FIELDS[3]}"
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[4]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${app_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board}; log_errors make install_${app_target} -j)
    if [ "$run_fs_target" = "Yes" ]; then
        echo '======================================================'
        echo '= Making filesystem for' ${app_target}
        echo '======================================================'
        (cd ${path}/build_${board}; log_errors make make_fs_${app_target} -j)
        (cd ${path}/build_${board}; cp ${app_target}_fat.fs ${DIST_DIR})
    fi
    if [ "$run_swmem_target" = "Yes" ]; then
        echo '======================================================'
        echo '= Making swmem for' ${app_target}
        echo '======================================================'
        (cd ${path}/build_${board}; log_errors make make_swmem_${app_target} -j)
        (cd ${path}/build_${board}; cp ${app_target}.swmem ${DIST_DIR})
    fi
done
