#!/bin/bash
set -e

if [ -z ${XCORE_SDK_PATH} ]; then
	echo "XCORE_SDK_PATH must be set before running this script."
else
    AI_PATH=${XCORE_SDK_PATH}/tools/ai
    AI_TOOLS_PATH=${XCORE_SDK_PATH}/tools/ai/ai_tools
    AI_TOOLS_SHA=cb81a22ba82a06a14dc210799589a24c00c2e24a

    LIB_FLEXBUFFERS_PATH=${AI_TOOLS_PATH}/utils/lib_flexbuffers
    XCORE_INTERPRETERS_PATH=${AI_TOOLS_PATH}/utils/adf/xcore_interpreters/host_library

    echo "*******************************"
    echo "* Cloning AI tools repository *"
    echo "*******************************"
    (cd ${AI_PATH}; git clone git@github.com:xmos/ai_tools.git)
    (cd ${AI_TOOLS_PATH}; git checkout ${AI_TOOLS_SHA})

    echo "**********************************"
    echo "* Updating AI tools submodules   *"
    echo "*                                *"
    echo "* This will take several minutes *"
    echo "**********************************"
    (cd ${AI_TOOLS_PATH}; git submodule update --init --recursive)

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

    echo "**********************************"
    echo "* Installing Python requirements *"
    echo "**********************************"
    pip install -r ${AI_PATH}/requirements.txt

    echo "**************************************"
    echo "* Updating PATH environment variable *"
    echo "**************************************"
    # setup conda to source the SetEnv on activate
    mkdir -p ${CONDA_PREFIX}/etc/conda/activate.d
    touch ${CONDA_PREFIX}/etc/conda/activate.d/env_vars.sh
    echo "source ${AI_PATH}/SetEnv" > ${CONDA_PREFIX}/etc/conda/activate.d/env_vars.sh
    # source the SetEnv so the env is setup now
    source ${AI_PATH}/SetEnv
fi