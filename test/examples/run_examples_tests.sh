#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

#**************************
# Run bare-metal tests
#**************************
echo "****************"
echo "* hello_world  *"
echo "****************"
(cd ${XCORE_SDK_ROOT}/examples/bare-metal/hello_world/test; bash hello_world_binary_test.sh) 

echo "****************"
echo "* micro_speech *"
echo "****************"
(cd ${XCORE_SDK_ROOT}/examples/bare-metal/micro_speech/bin; xrun --io micro_speech_test.xe) 

echo "****************"
echo "* cifar10      *"
echo "****************"
(cd ${XCORE_SDK_ROOT}/examples/bare-metal/cifar10/test; bash cifar10_binary_test.sh) 
