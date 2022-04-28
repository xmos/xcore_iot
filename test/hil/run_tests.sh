#!/bin/bash

declare -a hil_test_libs=(
    "lib_i2c"
    "lib_i2s"
    # "lib_qspi_io"
    "lib_spi"
    "lib_uart"
    )

#****************************
# Run tests and copy results
#****************************
tests_start=`date +%s`

for lib in ${hil_test_libs[@]}; do
    pushd .
    echo "************************"
    echo "* Building ${lib} tests *"
    echo "************************"
    ./build_${lib}_tests.sh
    echo "************************"
    echo "* Running ${lib} tests *"
    echo "************************"
    cd ${lib} && pytest --junitxml="test_results.xml"
    popd
done

tests_end=`date +%s`

#****************************
# Check results
#****************************
pytest test_verify_results.py

#****************************
# Display time results
#****************************
echo "************************"
echo "Test runtime: $((tests_end-tests_start))s"
echo "************************"
