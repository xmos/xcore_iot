cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_AIF TRUE)
set(USE_DISPATCHER TRUE)
set(USE_DEVICE_MEMORY_SUPPORT TRUE)

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(person_detect)
set(PROJECT_VERSION 0.6.0)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-Wxcore-fptrgroup"
    "-Wno-unknown-pragmas"
    "-mcmodel=large"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "app/main.c"
    "app/board_init.c"
    "app/ov2640/ov2640.c"
    "app/ov2640/ov2640_progs.c"
    "app/spi_camera/spi_camera.c"
    "app/model_runner/person_detect_model_runner.cc"
    "app/model_runner/person_detect_model_data.c"
    "app/model_runner/person_detect_task.c"
)
set(APP_INCLUDES
    "app"
    "app/ov2640"
    "app/spi_camera"
    "app/model_runner"
)

add_compile_definitions(
    OUTPUT_IMAGE_STREAM=1
    DEBUG_PRINT_ENABLE=1
    TF_LITE_STATIC_MEMORY=1
    XCORE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

if(${USE_EXTMEM})
    add_compile_definitions(USE_EXTMEM=1)
endif()

## Set any additional flags only for C++
set(CMAKE_CXX_FLAGS  "-std=c++11")

if(DEFINED THIS_XCORE_TILE)
    set(TARGET_NAME "${PROJECT_NAME}_${THIS_XCORE_TILE}.xe")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")

    add_compile_definitions(THIS_XCORE_TILE=${THIS_XCORE_TILE})
else()
    set(TARGET_NAME "${PROJECT_NAME}.xe")
endif()

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE
    ${APP_SOURCES}
    ${XMOS_RTOS_PLATFORM_SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${APP_INCLUDES}
    ${XMOS_RTOS_PLATFORM_INCLUDES}
)

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
