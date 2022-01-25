#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing
mkdir testing
REPORT=testing/test.rpt
FIRMWARE=clock_control_test.xe
TIMEOUT_S=60

rm -f ${REPORT}

echo "****************"
echo "* Run Tests    *"
echo "****************"
if [ "$UNAME" == "Linux" ] ; then
    timeout ${TIMEOUT_S}s xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT}
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xrun --xscope bin/${FIRMWARE} 2>&1 | tee -a ${REPORT}
fi

echo "****************"
echo "* Parse Result *"
echo "****************"

echo "**Currently Not Implemented**"
