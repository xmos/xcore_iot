#!/bin/bash
set -e

if [ -z ${XCORE_SDK_PATH} ]; then
	echo "XCORE_SDK_PATH must be set before running this script."
else
    AI_PATH=${XCORE_SDK_PATH}/tools/ai
    AI_TOOLS_PATH=${XCORE_SDK_PATH}/tools/ai/ai_tools

    LIB_FLEXBUFFERS_PATH=${AI_TOOLS_PATH}/utils/lib_flexbuffers
    XCORE_INTERPRETERS_PATH=${AI_TOOLS_PATH}/utils/adf/xcore_interpreters/host_library

    echo "****************************"
    echo "* Building libtflite2xcore *"
    echo "****************************"
    (cd ${LIB_FLEXBUFFERS_PATH}; mkdir -p build)
    (cd ${LIB_FLEXBUFFERS_PATH}/build; cmake ../ ; make install)

    echo "*******************************"
    echo "* Building xcore_interpreters *"
    echo "******************************"
    (cd ${XCORE_INTERPRETERS_PATH}; mkdir -p build)
    (cd ${XCORE_INTERPRETERS_PATH}/build; cmake ../ ; make install)
fi