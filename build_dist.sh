#!/bin/bash
set -e

#********************************
# Build & install lib_flexbuffers
#********************************
LIB_FLEXBUFFERS_DIR=tools/ai_tools/utils/lib_flexbuffers
echo "****************************"
echo "* Building libtflite2xcore *"
echo "****************************"
(cd ${LIB_FLEXBUFFERS_DIR}; mkdir -p build) 
(cd ${LIB_FLEXBUFFERS_DIR}/build; cmake ../ ; make install) 

#***********************************
# Build & install xcore_interpreters
#***********************************
XCORE_INTERPRETERS_DIR=tools/ai_tools/xcore_interpreters/python_bindings
echo "*******************************"
echo "* Building xcore_interpreters *"
echo "******************************"
(cd ${XCORE_INTERPRETERS_DIR}; mkdir -p build) 
(cd ${XCORE_INTERPRETERS_DIR}/build; cmake ../ ; make install) 

