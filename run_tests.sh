#!/bin/bash
set -e

#**************************
# Run bare-metal tests
#**************************
echo "****************"
echo "* hello_world  *"
echo "****************"
(cd examples/bare-metal/hello_world/test; bash hello_world_binary_test.sh) 


echo "****************"
echo "* micro_speech *"
echo "****************"
(cd examples/bare-metal/micro_speech/bin; xrun --io micro_speech_test.xe) 
