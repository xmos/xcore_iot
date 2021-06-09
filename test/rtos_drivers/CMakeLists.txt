cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_WIFI_MANAGER TRUE)
set(USE_DHCPD TRUE)
set(USE_FATFS TRUE)
set(USE_HTTP_CORE TRUE)
set(USE_JSON_PARSER TRUE)
set(USE_MQTT TRUE)
set(USE_SNTPD TRUE)
set(USE_TLS_SUPPORT TRUE)

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(rtos_drivers)
set(PROJECT_VERSION 0.6.0)

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
    "src/board_init.c"
)

set(APP_INCLUDES
    "src"
)

add_compile_definitions(
    RUN_INTERTILE_TESTS=${INTERTILE_TEST}
    RUN_I2C_TESTS=${I2C_TEST}
    RUN_GPIO_TESTS=${GPIO_TEST}
    RUN_SWMEM_TESTS=${SWMEM_TEST}
    RUN_QSPI_FLASH_TESTS=${QSPI_FLASH_TEST}
    RUN_I2S_TESTS=${I2S_TEST}
    DEBUG_PRINT_ENABLE_RTOS_I2S=1

    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

if(DEFINED THIS_XCORE_TILE)
    set(TARGET_NAME "${PROJECT_NAME}_${THIS_XCORE_TILE}.xe")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")

    add_compile_definitions(THIS_XCORE_TILE=${THIS_XCORE_TILE})
else()
    set(TARGET_NAME "${PROJECT_NAME}.xe")
endif()

## Gather test sources
file(GLOB_RECURSE INDIVIDUAL_TEST_SOURCES "src/individual_tests/*.c")

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES} ${INDIVIDUAL_TEST_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
