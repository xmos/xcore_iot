#!/bin/bash
set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing
mkdir testing
REPORT=testing/test.rpt
FIRMWARE=rtos_drivers_adv.xe
TIMEOUT_S=60

rm -f ${REPORT}

echo "****************"
echo "* Flash Device *"
echo "****************"
pushd .
cd filesystem_support
./flash_image.sh
popd

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
python ${XCORE_SDK_PATH}/test/rtos_drivers_adv/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose

pytest
