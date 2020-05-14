cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")

## Define project
project(app_freertos_explorer_board)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
		"-D__XS2A__=1"
		"-DMIC_DUAL_ENABLED=1"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "-DSL_WFX_DEBUG_MASK=SL_WFX_DEBUG_ERROR"
        "-DSL_WFX_DEFAULT_REQUEST_TIMEOUT_MS=500"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_C_SRCS
        "src/main.c"
        "src/audio_hw_config/audio_hw_config.c"
        "src/audio_pipeline/audio_pipeline.c"
        "src/gpio_ctrl/gpio_ctrl.c"
        "src/NetworkManager/network.c"
        "src/queue_to_i2s/queue_to_i2s.c"
        "src/queue_to_tcp_stream/queue_to_tcp_stream.c"
        "src/thruput_test/thruput_test.c"
        "src/udp_cli/CLI-commands.c"
        "src/udp_cli/FreeRTOS_CLI.c"
        "src/udp_cli/UDPCommandServer.c"
        "src/dhcpd.c"
        "src/afr_wifi/sl_wfx_callbacks.c"
        "src/afr_wifi/sl_wfx_iot_wifi.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
        "src/audio_hw_config"
        "src/audio_pipeline"
        "src/gpio_ctrl"
        "src/NetworkManager"
        "src/queue_to_i2s"
        "src/queue_to_tcp_stream"
        "src/thruput_test"
        "src/udp_cli"
        "src/afr_wifi"
        "src/afr_wifi/types"
    )
set(APP_DEPENDENT_MODULES
        "FreeRTOS-Plus-TCP(>=10.2.1)"
    )

set(USE_FREERTOS_SMP True)

## Add lib_soc devices
set(LIB_SOC_GPIO ${FULL})
set(LIB_SOC_I2C ${FULL})
set(LIB_SOC_I2S ${FULL})
set(LIB_SOC_MICARRAY ${FULL})
set(LIB_SOC_SPIMASTER ${FULL})
set(LIB_SOC_QSPI_FLASH ${FULL})
set(LIB_SOC_WFM200 ${BSP_ONLY})

## Register the application
XMOS_REGISTER_APP()
