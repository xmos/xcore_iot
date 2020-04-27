cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")

## Define project
project(app_freertos_xcoreai_test)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(C ASM)

## Set XMOS application variables
set(APP_HW_TARGET src/XCORE-AI-EXPLORER.xn)
set(APP_COMPILER_FLAGS
        "-D__XS2A__"
        "-O2"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-lquadflash"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_C_SRCS
        "src/main.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
    )
set(APP_DEPENDENT_MODULES
        "FreeRTOS(>=10.3.0)"
    )

set(USE_FREERTOS_SMP False)
set(FREERTOS_PORT "XCOREAI")

## Register the application
XMOS_REGISTER_APP()
