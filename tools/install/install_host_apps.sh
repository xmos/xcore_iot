#!/bin/sh

# Build program for fresh executables
# Get paths
PATH_INSTALL_SCRIPT=$PWD
PATH_XCORE_SDK_REPO=$(git rev-parse --show-toplevel)
PATH_HOSTAPP=$PATH_XCORE_SDK_REPO/modules/rtos/sw_services/fatfs/host

cd ${PATH_HOSTAPP}
rm -rf bin
rm -rf build
cmake -B build
(cd build ; make -j)
cd ${PATH_INSTALL_SCRIPT}

# alert user where it is
echo
echo
echo "Utility executables created at /opt/xmos/SDK/version/bin/"
echo "Please add to the PATH system environment variable."
echo
echo "Then, test with: fatfs_mkimage --help"
