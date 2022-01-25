cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration

## Import XMOS configurations
if(DEFINED XCORE_SDK_PATH)
    include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")
else()
    include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")
endif()

## Define project
project(clock_control_test)

## Enable languages for project
enable_language(CXX C ASM)

set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/platform/driver_instances.c"
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    ${XMOS_RTOS_PLATFORM_SOURCES}
)

set(APP_INCLUDES
    "src"
    "src/platform"
    ${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(RTOS_TILE_LIST 0 1)
create_multitile_target(RTOS_TILE_LIST)
