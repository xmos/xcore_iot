#!/bin/bash
set -e

declare -a hil_test_libs=(
    "lib_i2c"
    # "lib_i2s"
    # "lib_qspi_io"
    # "lib_spi"
    )

#**************************
# Build
#**************************
echo "******************"
echo "* Build apps     *"
echo "******************"
rm -rf build
cmake -B build
pushd .
cd build && make -j && make install
popd

#**************************
# Setup
#**************************
echo "******************"
echo "* Setup xmostest *"
echo "******************"
export PYTHONPATH=$PYTHONPATH:$XMOS_AIOT_SDK_PATH/test/hil/build/tools_xmostest/lib/python

#**************************
# Run tests
#**************************
for lib in ${hil_test_libs[@]}; do
    pushd .
    echo "************************"
    echo "* Running ${lib} tests *"
    echo "************************"
    cd ${lib} && python2 runtests.py
    popd
done
