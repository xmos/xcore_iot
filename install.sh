#!/bin/bash
set -e

if [ -z ${XMOS_AIOT_SDK_PATH} ]; then
	echo "XMOS_AIOT_SDK_PATH must be set before running this script."
else
    INSTALL_DIR=${XMOS_AIOT_SDK_PATH}/tools/install
    AI_TOOLS_DIR=${XMOS_AIOT_SDK_PATH}/tools/ai_tools

    echo "*****************************"
    echo "* Updating tools submodules *"
    echo "*****************************"
    (git submodule update --init --recursive ${AI_TOOLS_DIR})

    echo "****************************"
    echo "* Building libtflite2xcore *"
    echo "****************************"
    LIB_FLEXBUFFERS_DIR=${AI_TOOLS_DIR}/utils/lib_flexbuffers
    (cd ${LIB_FLEXBUFFERS_DIR}; mkdir -p build)
    (cd ${LIB_FLEXBUFFERS_DIR}/build; cmake ../ ; make install)

    AIF_DIR=${XMOS_AIOT_SDK_PATH}/modules/aif

    echo "**********************************"
    echo "* Installing Python requirements *"
    echo "**********************************"
    pip install -r ${INSTALL_DIR}/requirements.txt

    echo "**************************************"
    echo "* Updating PATH environment variable *"
    echo "**************************************"
    # setup conda to source the SetEnv on activate
    mkdir -p ${CONDA_PREFIX}/etc/conda/activate.d
    touch ${CONDA_PREFIX}/etc/conda/activate.d/env_vars.sh
    echo "source ${INSTALL_DIR}/SetEnv" > ${CONDA_PREFIX}/etc/conda/activate.d/env_vars.sh
    # source the SetEnv so the env is setup now
    source ${INSTALL_DIR}/SetEnv
fi