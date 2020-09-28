#!/bin/bash
set -e

#********************************
# Build & install libtflite2xcore
#********************************
LIBTFLITE2XCORE_DIR=tools/ai_tools/utils/python_bindings
echo "****************************"
echo "* Building libtflite2xcore *"
echo "****************************"
(cd ${LIBTFLITE2XCORE_DIR}; mkdir -p build) 
(cd ${LIBTFLITE2XCORE_DIR}/build; cmake ../ ; make) 

echo "******************************"
echo "* Installing libtflite2xcore *"
echo "******************************"
(cd ${LIBTFLITE2XCORE_DIR}/build; make install) 
