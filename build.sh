#!/bin/bash
set -e

#**************************
# Build bare-metal examples
#**************************
declare -a bare_metal_examples=(
    "cifar10"
    "hello_world"
    "micro_speech"
    "mobilenet_v1"
    )
 
for example in ${bare_metal_examples[@]}; do
    (cd examples/bare-metal/${example}; mkdir -p build) 
    (cd examples/bare-metal/${example}/build; cmake ../ ; make install) 
done

#**************************
# Build FreeRTOS examples
#**************************
BOARD="-DBOARD=XCORE-AI-EXPLORER"

declare -a freertos_examples=(
    "cifar10"
    "explorer_board"
    "independent_tiles"
    "iot_aws"
    "person_detection"
    )

for example in ${freertos_examples[@]}; do
    (cd examples/freertos/${example}; mkdir -p build) 
    (cd examples/freertos/${example}/build; cmake ../ ${BOARD}; make) 
done
