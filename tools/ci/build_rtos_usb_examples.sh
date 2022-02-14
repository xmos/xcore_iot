#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD"
    demos=(
        "example_freertos_usb_tusb_demo_audio_test XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_cdc_dual_ports XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_cdc_msc XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_dfu_runtime XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_hid_composite XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_hid_generic_inout XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_hid_multiple_interface XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_midi_test XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_msc_dual_lun XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_usbtmc XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_webusb_serial XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    demos=(
        "example_freertos_usb_tusb_demo_audio_test XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_dfu_runtime XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_midi_test XCORE-AI-EXPLORER"
        "example_freertos_usb_tusb_demo_hid_generic_inout XCORE-AI-EXPLORER"
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
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${demo} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DBOARD=${board}; log_errors make ${demo} -j)
done
