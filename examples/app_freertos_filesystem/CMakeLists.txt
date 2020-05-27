cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")

## Define project
project(app_freertos_filesystem)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "-DSL_WFX_DEBUG_MASK=SL_WFX_DEBUG_ERROR"
        "-DSL_WFX_DEFAULT_REQUEST_TIMEOUT_MS=5000"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_C_SRCS
        "src/main.c"
        "src/network.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
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
set(LIB_SOC_QSPI_FLASH ${FULL})
set(LIB_SOC_SW_FATFS True)
set(LIB_SOC_SW_WIFI True)
set(LIB_SOC_SW_DHCPD True)

## Register the application
XMOS_REGISTER_APP()
