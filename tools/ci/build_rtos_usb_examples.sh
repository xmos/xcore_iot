#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_SDK_ROOT}/dist
mkdir -p ${DIST_DIR}

# row format is: "make_target board toolchain"
demos=(
    "example_freertos_usb_tusb_demo_audio_test              XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_cdc_dual_ports          XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_cdc_msc                 XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_dfu                     XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    # "example_freertos_usb_tusb_demo_dfu_runtime             XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_hid_boot_interface      XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_hid_composite           XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_hid_generic_inout       XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_hid_multiple_interface  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_midi_test               XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_msc_dual_lun            XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_uac2_headset            XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_usbtmc                  XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_video_capture           XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
    "example_freertos_usb_tusb_demo_webusb_serial           XCORE-AI-EXPLORER  xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#demos[@]}; i += 1)); do
    read -ra FIELDS <<< ${demos[i]}
    make_target="${FIELDS[0]}"
    board="${FIELDS[1]}"
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[2]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${make_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board}; log_errors make ${make_target} -j)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR})
done
