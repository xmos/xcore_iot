cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../../../rtos_support")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")
set(XMOS_APP_MODULES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(AI_TOOLS_DIR "../../../../ai_tools")

## Define project
project(app_freertos_cifar10)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-report"
        "-fxscope"
		"-DMIC_DUAL_ENABLED=1"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "-Wno-unknown-pragmas"
        "-DTF_LITE_STATIC_MEMORY"
        "-D_XCORE_HAS_SWMEM"
        "-DTF_LITE_STRIP_ERROR_STRINGS"
        "-DNDEBUG"
        "-mcmodel=large"
        "-DXCORE"
        "${CMAKE_CURRENT_SOURCE_DIR}/app/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_CXX_SRCS
        ""
    )
set(APP_C_SRCS
        "app/main.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "app"
    )
set(APP_DEPENDENT_MODULES
        "lib_soc(>=0.6.0)"
        "inference_engine(>=0.0.0)"
    )

list(APPEND APP_COMPILER_FLAGS "-DUSE_SWMEM" "-DUSE_QSPI_SWMEM_DEV=1")
# list(APPEND APP_COMPILER_FLAGS "-DUSE_EXTMEM")

## Set any additional flags only for C++
set(CMAKE_CXX_FLAGS  "-std=c++11")

set(USE_FREERTOS_SMP True)

## Add lib_soc devices
set(LIB_SOC_GPIO ${FULL})
set(LIB_SOC_I2C ${BITSTREAM_ONLY})
set(LIB_SOC_I2S ${BITSTREAM_ONLY})
set(LIB_SOC_MICARRAY ${BITSTREAM_ONLY})
set(LIB_SOC_SPIMASTER ${BITSTREAM_ONLY})
set(LIB_SOC_QSPI_FLASH ${FULL})
set(LIB_SOC_AI ${FULL})
set(LIB_SOC_SW_FATFS True)

## Register the application
XMOS_REGISTER_APP()
