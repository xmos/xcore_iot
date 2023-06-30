#!/bin/bash
set -e

XCORE_IOT_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_IOT_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_IOT_ROOT}/dist_host
mkdir -p ${DIST_DIR}

# row format is: "target     copy_path"
applications=(
    "example_freertos_device_control_host   examples/freertos/device_control/host"
    "fatfs_mkimage              modules/rtos/tools/fatfs_mkimage"
    "datapartition_mkimage      modules/rtos/tools/datapartition_mkimage"
    "xscope_host_endpoint                   modules/xscope_fileio/xscope_fileio/host"
    "xscope2psf                             examples/freertos/tracealyzer/host"
)

# perform builds
path="${XCORE_IOT_ROOT}"
echo '******************************************************'
echo '* Building host applications'
echo '******************************************************'

(cd ${path}; rm -rf build_host)
(cd ${path}; mkdir -p build_host)
(cd ${path}/build_host; log_errors cmake ../)

for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    target="${FIELDS[0]}"
    copy_path="${FIELDS[1]}"
    (cd ${path}/build_host; log_errors make ${target} -j)
    (cd ${path}/build_host; cp ${copy_path}/${target} ${DIST_DIR})
done
