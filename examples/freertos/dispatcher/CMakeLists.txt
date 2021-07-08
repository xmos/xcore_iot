cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_DISPATCH_QUEUE TRUE)

## Import XMOS RTOS platform configuration.
## Must be done after setting the configuration options.
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(dispatch_queue)
set(PROJECT_VERSION 0.5.0)

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
)

set(APP_INCLUDES
    "src"
)

set(SYSCLK_FREQ 700)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=0
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
