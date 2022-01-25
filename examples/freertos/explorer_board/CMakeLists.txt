cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_FATFS TRUE)

## This app only supports the XCORE-AI-EXPLORER board
set(BOARD XCORE-AI-EXPLORER)

#**********************
# Get path to XCore SDK
#**********************
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

## Import XMOS configurations
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

## Define project
project(explorer_board)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-lquadspi"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/platform/aic3204.c"
    "src/platform/app_pll_ctrl.c"
    "src/platform/driver_instances.c"
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/example_pipeline.c"
    "src/gpio_ctrl/gpio_ctrl.c"
    "src/mem_analysis/mem_analysis.c"
    "src/filesystem/filesystem_demo.c"
    ${XMOS_RTOS_PLATFORM_SOURCES}
)

set(APP_INCLUDES
    "src"
    ${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(RTOS_TILE_LIST 0 1 )
create_multitile_target(RTOS_TILE_LIST)
