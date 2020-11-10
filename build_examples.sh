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
    echo '***************************'
    echo '* Building' ${example}
    echo '***************************'
    (cd examples/bare-metal/${example}; rm -rf build)
    (cd examples/bare-metal/${example}; mkdir -p build)
    (cd examples/bare-metal/${example}/build; cmake ../ ; make install)
done

#**************************
# Build FreeRTOS examples
#**************************
BOARD=XCORE-AI-EXPLORER

declare -a freertos_examples=(
    "cifar10"
    "explorer_board"
    "iot_aws"
    "person_detection"
    )

for example in ${freertos_examples[@]}; do
    echo '***************************'
    echo '* Building' ${example}
    echo '***************************'
    (cd examples/freertos/${example}; rm -rf build)
    (cd examples/freertos/${example}; mkdir -p build)
    (cd examples/freertos/${example}/build; cmake ../ -DBOARD=${BOARD}; make)
done

declare -a freertos_make_examples=(
    "independent_tiles"
    )

for example in ${freertos_make_examples[@]}; do
    echo '***************************'
    echo '* Building' ${example}
    echo '***************************'
    (cd examples/freertos/${example}; make BOARD=${BOARD})
done
