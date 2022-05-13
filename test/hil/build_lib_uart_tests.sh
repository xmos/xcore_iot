#!/bin/bash
set -e

#Build tests in parallel. Quite tough on host machine (200+ processes) but saves ~3x time
parallel=1

XCORE_SDK_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_SDK_ROOT}/tools/ci/helper_functions.sh

# setup configurations
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "make_target BOARD toolchain"
    applications=(

###################################  FIFO  #############################################################
"test_hil_uart_fifo_test XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"

################################### UART TX ############################################################
"test_hil_uart_tx_test_UNBUFFERED_921600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_921600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_576000_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_115200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_UNBUFFERED_9600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_tx_test_BUFFERED_921600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_576000_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_115200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_tx_test_BUFFERED_9600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake" 


################################### UART RX ############################################################
"test_hil_uart_rx_test_UNBUFFERED_921600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_921600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_576000_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_115200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_UNBUFFERED_9600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
# "test_hil_uart_rx_test_BUFFERED_921600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_576000_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_115200_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_8_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_NONE_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_EVEN_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_ODD_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
"test_hil_uart_rx_test_BUFFERED_9600_5_ODD_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"


)
elif [ "$1" == "smoke" ]
then
    applications=(
        #Pick the fastest options to test (good for runtime too). One vanilla 8b N 1 and one funky one
        "test_hil_uart_rx_test_BUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
        "test_hil_uart_rx_test_UNBUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"

        "test_hil_uart_tx_test_BUFFERED_576000_8_NONE_1 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
        "test_hil_uart_tx_test_UNBUFFERED_921600_5_EVEN_2 XCORE-AI-EXPLORER tools/xmos_cmake_toolchain/xs3a.cmake"
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
   if [ "$parallel" == "0" ]
   then
        (cd ${path}/build_ci_${application}_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application} -j)
   else
        (cd ${path}/build_ci_${application}_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DXCORE_SDK_CI_TESTING=ON; log_errors make ${application}) 
   fi
   (cd ${path}; rm -rf build_ci_${application}_${board})
}


# run build processes and store pids in array
for ((i = 0; i < ${#applications[@]}; i += 1)); do

    if [ "$parallel" == "0" ]
    then
        do_build "$i"
    else
        do_build "$i" &
        pids[${i}]=$!
        sleep 0.1 #Limit rate of spawning a bit to make life easier for host
    fi
done

if [ "$parallel" != "0" ]
then
    # wait for all pids
    for pid in ${pids[*]}; do
        wait $pid
    done
fi