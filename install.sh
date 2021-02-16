#!/bin/bash
set -e

if [ -z ${XMOS_AIOT_SDK_PATH} ]; then
	echo "XMOS_AIOT_SDK_PATH must be set before running this script."
else
    AI_TOOLS_DIR=${XMOS_AIOT_SDK_PATH}/tools/ai_tools

    echo "*****************************"
    echo "* Updating tools submodules *"
    echo "*****************************"
    (cd ${AI_TOOLS_DIR}; git submodule update --init) 

    echo "****************************"
    echo "* Building libtflite2xcore *"
    echo "****************************"
    LIB_FLEXBUFFERS_DIR=${AI_TOOLS_DIR}/utils/lib_flexbuffers
    (cd ${LIB_FLEXBUFFERS_DIR}; mkdir -p build)
    (cd ${LIB_FLEXBUFFERS_DIR}/build; cmake ../ ; make install)

    ADF_DIR=${XMOS_AIOT_SDK_PATH}/modules/adf

    echo "*******************************"
    echo "* Building xcore_interpreters *"
    echo "******************************"
    XCORE_INTERPRETERS_DIR=${ADF_DIR}/xcore_interpreters/python_bindings
    (cd ${XCORE_INTERPRETERS_DIR}; mkdir -p build)
    (cd ${XCORE_INTERPRETERS_DIR}/build; cmake ../ ; make install)

    echo "**********************************"
    echo "* Installing Python requirements *"
    echo "**********************************"
    pip install -r requirements.txt

    echo "**************************************"
    echo "* Updating PATH environment variable *"
    echo "**************************************"
    cd $CONDA_PREFIX
    mkdir -p ./etc/conda/activate.d
    touch ./etc/conda/activate.d/env_vars.sh
    echo "PATH=${XMOS_AIOT_SDK_PATH}/modules/adf/tools/generate:${PATH}" > ./etc/conda/activate.d/env_vars.sh
    cd ..
    echo ""
    echo "Deactivate and activate your environment to complete the install:"
    echo "$ conda deactivate"
    echo "$ conda activate ${CONDA_PREFIX}"
fi
