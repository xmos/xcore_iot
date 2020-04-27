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

#set(FREERTOS_PORT "XCORE200")
set(FREERTOS_PORT "XCOREAI")

## Set XMOS application variables

set(APP_COMPILER_FLAGS
        "-Os"
        "-D__XS2A__=1" # Hack to get some libs to work with XS3
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-lquadflash"
        "-DDEBUG_PRINT_ENABLE=1"
        "-DSL_WFX_DEBUG_MASK=SL_WFX_DEBUG_INIT"
        "-DSL_WFX_DEFAULT_REQUEST_TIMEOUT_MS=500"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")

if(FREERTOS_PORT STREQUAL "XCOREAI")
    set(bitstream_dir "xcoreai_explorer_bitstream_src")
    set(APP_HW_TARGET "${bitstream_dir}/XCORE-AI-EXPLORER.xn")
    list(APPEND APP_COMPILER_FLAGS "-DXCOREAI_EXPLORER=1" "-mcmodel=large")
elseif(FREERTOS_PORT STREQUAL "XCORE200")
    set(bitstream_dir "bitstream_src")
    set(APP_HW_TARGET "${bitstream_dir}/MIC-ARRAY-1V3.xn")
    list(APPEND APP_COMPILER_FLAGS "-DXCORE200_MAB=1")
else()
    message(SEND_ERROR "Invalid FreeRTOS port ${FREEROTS_PORT}")
endif()

set(APP_XC_SRCS
        "${bitstream_dir}/bitstream.xc"
    )
set(APP_C_SRCS
        "src/main.c"
        "src/network.c"
        "src/dhcpd.c"
        "src/afr_wifi/sl_wfx_callbacks.c"
        "src/afr_wifi/sl_wfx_iot_wifi.c"
        "${bitstream_dir}/bitstream.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
        "src/afr_wifi"
        "src/afr_wifi/types"
        "${bitstream_dir}"
    )
set(APP_DEPENDENT_MODULES
        "FreeRTOS-Plus-TCP(>=10.3.0)"
    )

set(USE_FREERTOS_SMP False)

## Add lib_soc devices
set(LIB_SOC_GPIO ${FULL})
set(LIB_SOC_SPIMASTER ${FULL})
set(LIB_SOC_WFM200 ${BSP_ONLY})
set(LIB_SOC_MICARRAY ${FULL})
set(LIB_SOC_I2S ${FULL})
set(LIB_SOC_I2C ${FULL})

## Register the application
XMOS_REGISTER_APP()
