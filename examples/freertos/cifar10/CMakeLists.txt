cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(MULTITILE_BUILD TRUE)
set(USE_FATFS TRUE)
set(USE_AIF TRUE)
set(USE_DISPATCHER TRUE)

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

## Define project
project(cifar10)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-lquadspi"
    "-Wxcore-fptrgroup"
    "-Wno-unknown-pragmas"
    "-mcmodel=large"
    "-MD"
    "-MT"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

#**********************
# Additional support sources
#**********************
set(APP_SOURCES
    "src/main.c"
    "src/platform/driver_instances.c"
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/model_runner/cifar10_model_data.c"
    "src/model_runner/cifar10_model_runner.cc"
    "src/model_runner/cifar10_task.c"
    "${XCORE_SDK_PATH}/modules/device_memory_support/rtos/xcore_device_memory.c"
	${XMOS_RTOS_PLATFORM_SOURCES}
)
set(APP_INCLUDES
    "src"
    "src/model_runner"
    "${XCORE_SDK_PATH}/modules/device_memory_support/rtos"
	${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    TF_LITE_STATIC_MEMORY=1
    XCORE=1
)

if(${USE_SWMEM})
    add_compile_definitions(USE_SWMEM=1)
elseif(${USE_EXTMEM})
    add_compile_definitions(USE_EXTMEM=1)
endif()

## Set any additional flags only for C++
set(CMAKE_CXX_FLAGS  "-std=c++11")

set(RTOS_TILE_LIST 0 1 )
create_multitile_target(RTOS_TILE_LIST)
