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
(cd ${path}/build_host; log_errors cmake ../)

# example_freertos_device_control_host
name=device_control/host
make_target=example_freertos_device_control_host
(cd ${path}/build_host; log_errors make ${make_target} -j)
(cd ${path}/build_host; cp examples/freertos/${name}/${make_target} ${DIST_DIR})

# fatfs_mkimage
name=fatfs/host
make_target=fatfs_mkimage
(cd ${path}/build_host; log_errors make ${make_target} -j)
(cd ${path}/build_host; cp modules/rtos/modules/sw_services/${name}/${make_target} ${DIST_DIR})

# xscope_host_endpoint
name=xscope_fileio/host
make_target=xscope_host_endpoint
(cd ${path}/build_host; log_errors make ${make_target} -j)
(cd ${path}/build_host; cp modules/xscope_fileio/${name}/${make_target} ${DIST_DIR})
