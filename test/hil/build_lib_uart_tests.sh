#!/bin/bash
set -e

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD toolchain"
    applications=(
        # "test_hil_uart_fifo_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
        # "test_hil_uart_rx_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_1843200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_921600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_576000_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_115200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_buffer0_9600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake" 
)
elif [ "$1" == "smoke" ]
then
    applications=(
        # "test_hil_uart_rx_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
        # "test_hil_uart_tx_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
    )
else
    echo "Argument $1 not a supported configuration!"
    return
fi

do_build () {
   # echo "building: ${1}"
   read -ra FIELDS <<< ${applications[$1]}
   application="${FIELDS[0]}"
   board="${FIELDS[1]}"
   toolchain_file="${XCORE_SDK_ROOT}/${FIELDS[2]}"
   path="${XCORE_SDK_ROOT}"
   echo '******************************************************'
   echo '* Building' ${application} 'for' ${board}
   echo '******************************************************'

   (cd ${path}; rm -rf build_ci_${application}_${board})
   (cd ${path}; mkdir -p  build_ci_${application}_${board})
   (cd ${path}/build_ci_${application}_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application} -j)
   # (cd ${path}/build_ci_${application}_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application}) 
   # (cd ${path}; rm -rf build_ci_${application}_${board})
}


# run build processes and store pids in array
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    do_build "$i" &
    # do_build "$i"
    pids[${i}]=$!
    sleep 2 #Limit rate of spawning a bit
done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done