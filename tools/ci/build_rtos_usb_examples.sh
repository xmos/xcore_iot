#!/bin/bash
set -e

source ${XCORE_SDK_PATH}/tools/ci/helper_functions.sh

USB_EXAMPLE_PATH="examples/freertos/usb"

# setup configuraitons
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "DEMO BOARD"
    demos=(
        "AUDIO_TEST XCORE-AI-EXPLORER"
        "HID_COMPOSITE_TEST XCORE-AI-EXPLORER"
        "WEBUSB_SERIAL XCORE-AI-EXPLORER"
        "MIDI_TEST XCORE-AI-EXPLORER"
        "USBTMC XCORE-AI-EXPLORER"
        "CDC_MSC_TEST XCORE-AI-EXPLORER"
        "MSC_DUAL_LUN XCORE-AI-EXPLORER"
        "DFU_RUNTIME_TEST XCORE-AI-EXPLORER"
        "CDC_DUAL_PORTS_TEST XCORE-AI-EXPLORER"
        "HID_GENERIC_INOUT_TEST XCORE-AI-EXPLORER"
        "HID_MULTIPLE_INTERFACE_TEST XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    demos=(
        "AUDIO_TEST XCORE-AI-EXPLORER"
        "MIDI_TEST XCORE-AI-EXPLORER"
        "DFU_RUNTIME_TEST XCORE-AI-EXPLORER"
        "HID_GENERIC_INOUT_TEST XCORE-AI-EXPLORER"
    )
else 
    echo "Argument $1 not a supported configuration!"
    exit
fi

# perform builds
for ((i = 0; i < ${#demos[@]}; i += 1)); do
    read -ra FIELDS <<< ${demos[i]}
    demo="${FIELDS[0]}"
    board="${FIELDS[1]}"
    path="${XCORE_SDK_PATH}/${USB_EXAMPLE_PATH}"    
    echo '******************************************************'
    echo '* Building' ${demo} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; log_errors make distclean)
    (cd ${path}; log_errors make -j BOARD=${board} TINYUSB_DEMO=${demo})
done

