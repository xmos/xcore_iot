cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")

## Define project
project(app_freertos_wfm200)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(C ASM)

## Set XMOS application variables
set(APP_HW_TARGET bitstream_src/MIC-ARRAY-1V3.xn)
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-lquadflash"
        "-DDEBUG_PRINT_ENABLE=1"
        "-DSL_WFX_DEBUG_MASK=SL_WFX_DEBUG_INIT"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        "bitstream_src/bitstream.xc"
    )
set(APP_C_SRCS
        "src/main.c"
        "src/network.c"
        "src/afr_wifi/sl_wfx_callbacks.c"
        "src/afr_wifi/sl_wfx_iot_wifi.c"
        "bitstream_src/bitstream.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
        "src/afr_wifi"
        "src/afr_wifi/types"
        "bitstream_src"
    )
set(APP_DEPENDENT_MODULES
        "FreeRTOS-Plus-TCP(>=10.2.1)"
    )

set(USE_FREERTOS_SMP True)

## Add lib_soc devices
set(LIB_SOC_GPIO ${FULL})
set(LIB_SOC_SPIMASTER ${FULL})
set(LIB_SOC_WFM200 ${BSP_ONLY})
set(LIB_SOC_MICARRAY ${FULL})
set(LIB_SOC_I2S ${FULL})
set(LIB_SOC_I2C ${FULL})

## Register the application
XMOS_REGISTER_APP()
