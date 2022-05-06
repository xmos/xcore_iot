#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# row format is: "make_target BOARD toolchain"
applications=(
    "test_hil_backpressure_test_768000_4_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_4_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_4_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_3_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_3_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_3_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_2_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_2_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_2_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_1_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_1_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_768000_1_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_4_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_4_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_4_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_3_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_3_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_3_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_2_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_2_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_2_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_1_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_1_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_384000_1_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_4_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_4_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_4_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_3_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_3_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_3_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_2_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_2_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_2_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_1_5_5        XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_1_0_10       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_backpressure_test_192000_1_10_0       XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_0_4_4  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_0_1_1  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_0_4_0  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_0_0_4  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_1_4_4  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_1_1_1  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_1_4_0  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_external_clock_test_1_0_4  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_0_4_4                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_0_1_1                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_0_4_0                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_0_0_4                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_1_4_4                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_1_1_1                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_1_4_0                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_master_test_1_0_4                 XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_4_4                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_1_1                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_4_0                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_0_4                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_2_2                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_4_4                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_1_1                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_4_0                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_0_4                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_2_2                  XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_4_4_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_1_1_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_4_0_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_0_4_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_0_2_2_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_4_4_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_1_1_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_4_0_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_0_4_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
    "test_hil_i2s_slave_test_1_2_2_inv              XCORE-AI-EXPLORER  tools/xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    application="${FIELDS[0]}"
    board="${FIELDS[1]}"
    toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[2]}"
    path="${XCORE_SDK_ROOT}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_ci_${board})
    (cd ${path}; mkdir -p build_ci_${board})
    (cd ${path}/build_ci_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application} -j)
done
