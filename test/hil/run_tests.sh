#!/bin/bash
set -e

if [ -z "$1" ] || [ "$1" == "all" ]
then
    declare -a hil_test_libs=(
        "lib_uart"
        "lib_i2c"
        "lib_i2s"
        "lib_spi"
        #"lib_qspi_io"
        )
else
    declare -a hil_test_libs=(
        "$1"
        )
fi

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
pytest test_verify_results.py ${hil_test_libs[*]}

#****************************
# Display time results
#****************************
echo "************************"
echo "Test runtime: $((tests_end-tests_start))s"
echo "************************"
