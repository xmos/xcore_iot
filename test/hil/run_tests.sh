#!/bin/bash

declare -a hil_test_libs=(
    "lib_i2c"
    "lib_i2s"
    # "lib_qspi_io"
    "lib_spi"
    )

#****************************
# Build
#****************************
build_start=`date +%s`

echo "******************"
echo "* Build apps     *"
echo "******************"
rm -rf build
rm -rf results
mkdir results
cmake -B build
pushd .
cd build && make && make install
popd

build_end=`date +%s`

#****************************
# Setup
#****************************
echo "******************"
echo "* Setup xmostest *"
echo "******************"
export PYTHONPATH=$PYTHONPATH:$XCORE_SDK_PATH/test/hil/build/tools_xmostest/lib/python

#****************************
# Run tests and copy results
#****************************
tests_start=`date +%s`

for lib in ${hil_test_libs[@]}; do
    pushd .
    echo "************************"
    echo "* Running ${lib} tests *"
    echo "************************"
    cd ${lib} && python2 runtests.py
    popd
done

tests_end=`date +%s`

#****************************
# Check results
#****************************
pytest test_verify_i2c_results.py test_verify_i2s_results.py test_verify_spi_results.py

#****************************
# Display time results
#****************************
echo "************************"
echo "Build runtime: $((build_end-build_start))s  Test runtime: $((tests_end-tests_start))s"
echo "************************"
