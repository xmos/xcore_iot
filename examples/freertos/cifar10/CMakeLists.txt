cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_FATFS TRUE)
set(USE_MODEL_RUNNER TRUE)
set(USE_TENSORFLOW_SUPPORT TRUE)

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(cifar10)
set(PROJECT_VERSION 0.6.0)

## Enable languages for project
enable_language(CXX XC C ASM)

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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)
set(APP_SOURCES
    "src/main.c"
    "src/board_init.c"
    "src/model_runner/cifar10_model_data.c"
    "src/model_runner/cifar10_model_runner.cc"
    "src/model_runner/cifar10_task.c"
)
set(APP_INCLUDES
    "src"
    "src/model_runner"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    MIC_DUAL_ENABLED=1
    TF_LITE_STATIC_MEMORY=1
    # TF_LITE_STRIP_ERROR_STRINGS=1  # causes build issues???
    # NDEBUG=1  # causes build issues???
    XCORE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

# if(${USE_SWMEM})
#   add_compile_definitions(-DUSE_SWMEM -DUSE_QSPI_SWMEM_DEV=1)
# elseif(${USE_EXTMEM})
  # add_compile_definitions(USE_EXTMEM=1)
# endif()

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

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
