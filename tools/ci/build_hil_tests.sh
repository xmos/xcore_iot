#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# perform builds
## TODO replace with build and running tests
path="${XCORE_SDK_ROOT}/test/hil"

echo '******************************************************'
echo '* Building HIL Tests'
echo '******************************************************'

(cd ${path}; ./build_lib_i2c_tests.sh)
(cd ${path}; ./build_lib_i2s_tests.sh)
(cd ${path}; ./build_lib_spi_tests.sh)
