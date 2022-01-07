#!/bin/sh

# build program for fresh executables
INSTALL_SCRIPT_PATH=$PWD
XCORE_SDK_REPO_PATH=$(git rev-parse --show-toplevel)
HOSTAPP_PATH=$XCORE_SDK_REPO_PATH/modules/rtos/sw_services/fatfs/host

echo $INSTALL_SCRIPT_PATH
echo $HOSTAPP_PATH
cd ${HOSTAPP_PATH}
rm -rf bin
rm -rf build
cmake -B build
(cd build ; make -j)
cd ${INSTALL_SCRIPT_PATH}

# alert user where it is
echo
echo
echo "Utility executables for fatfs_mkimage created at /opt/xmos/SDK/version/bin/."
echo "Please add to the PATH system environment variable."
echo
echo "Then, test with fatfs_mkimage.exe --help"
