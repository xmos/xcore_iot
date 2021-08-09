#!/bin/bash
set -e

if [ -z ${XCORE_SDK_PATH} ]; then
	echo "XCORE_SDK_PATH must be set before running this script."
else
    (pip install -r ${XCORE_SDK_PATH}/tools/install/requirements.txt)
    (pip install -r ${XCORE_SDK_PATH}/tools/install/contribute.txt)
    (pip install -r ${XCORE_SDK_PATH}/documents/requirements.txt)
fi