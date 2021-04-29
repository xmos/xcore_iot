#!/bin/bash
set -e

declare -a boards=(
    "XCORE-AI-EXPLORER"
    "OSPREY-BOARD"
    )

#**************************
# Build bare-metal examples
#**************************
declare -a bare_metal_examples=(
    "micro_speech"
    )

for board in ${boards[@]}; do
    for example in ${bare_metal_examples[@]}; do
        echo '******************************************************'
        echo '* Building' ${example} 'for' ${board}
        echo '******************************************************'
        (cd examples/bare-metal/${example}; rm -rf build_${board})
        (cd examples/bare-metal/${example}; mkdir -p build_${board})
        (cd examples/bare-metal/${example}/build_${board}; cmake ../ -DBOARD=${board}; make -j install)
    done
done

#*************************************************
# Build bare-metal XCORE-AI-EXPLORER only examples
#*************************************************
board=XCORE-AI-EXPLORER
declare -a bare_metal_examples=(
    "cifar10"
    "hello_world"
    "visual_wake_words"
    "hotdog_not_hotdog"
    )

for example in ${bare_metal_examples[@]}; do
    echo '******************************************************'
    echo '* Building' ${example} 'for' ${board}
    echo '******************************************************'
    (cd examples/bare-metal/${example}; rm -rf build)
    (cd examples/bare-metal/${example}; mkdir -p build)
    (cd examples/bare-metal/${example}/build; cmake ../ ; make -j install)
done

#**************************
# Build FreeRTOS examples
#**************************
declare -a freertos_examples=(
    "cifar10"
    "usb"
    )

for board in ${boards[@]}; do
    for example in ${freertos_examples[@]}; do
        echo '******************************************************'
        echo '* Building' ${example} 'for' ${board}
        echo '******************************************************'
        (cd examples/freertos/${example}; make distclean)
        (cd examples/freertos/${example}; make -j BOARD=${board})
    done
done

#***********************************************
# Build FreeRTOS XCORE-AI-EXPLORER only examples
#***********************************************
board=XCORE-AI-EXPLORER

declare -a freertos_examples=(
    "explorer_board"
    "independent_tiles"
    "iot_aws"
    "person_detection"
    )

for example in ${freertos_examples[@]}; do
    echo '******************************************************'
    echo '* Building' ${example} 'for' ${board}
    echo '******************************************************'
    (cd examples/freertos/${example}; make distclean)
    (cd examples/freertos/${example}; make -j BOARD=${board})
done

#***********************************************
# Build FreeRTOS XCORE200-MIC-ARRAY only examples
#***********************************************
board=XCORE200-MIC-ARRAY

declare -a freertos_examples=(
    "device_control"
    )

for example in ${freertos_examples[@]}; do
    echo '******************************************************'
    echo '* Building' ${example} 'for' ${board}
    echo '******************************************************'
    (cd examples/freertos/${example}; make distclean)
    (cd examples/freertos/${example}; make -j BOARD=${board})
done

