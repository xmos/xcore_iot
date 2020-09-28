#!/bin/bash
set -e

#**************************
# Run bare-metal tests
#**************************
echo "****************"
echo "* micro_speech *"
echo "****************"
(cd examples/bare-metal/micro_speech/bin; xrun --io micro_speech_test.xe) 
