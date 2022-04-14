#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist_host
mkdir -p ${DIST_DIR}

# perform builds
path="${XCORE_SDK_ROOT}"
echo '******************************************************'
echo '* Building host applications'
echo '******************************************************'

(cd ${path}; rm -rf build_host)
(cd ${path}; mkdir -p build_host)
(cd ${path}/build_host; log_errors cmake ../ ; log_errors make -j)

# copy example_freertos_device_control_host to dist
name=device_control/host
make_target=example_freertos_device_control_host
(cd ${path}/build_host; cp examples/freertos/${name}/${make_target} ${DIST_DIR})

# copy fatfs_mkimage to dist
name=fatfs/host
make_target=fatfs_mkimage
(cd ${path}/build_host; cp modules/rtos/sw_services//${name}/${make_target} ${DIST_DIR})
