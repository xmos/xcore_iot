#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing
mkdir testing
REPORT=testing/test.rpt
FIRMWARE=test_rtos_driver_hil_add.xe
TIMEOUT_S=60

rm -f ${REPORT}

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

echo "****************"
echo "* Run Tests    *"
echo "****************"
if [ "$UNAME" == "Linux" ] ; then
    timeout ${TIMEOUT_S}s xrun --xscope ${XCORE_SDK_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope ${XCORE_SDK_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
python ${XCORE_SDK_ROOT}/test/rtos_drivers/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose

pytest
