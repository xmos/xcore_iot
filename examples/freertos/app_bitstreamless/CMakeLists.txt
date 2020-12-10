cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(rtos_app)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(XC CXX C ASM)

set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-D__XS2A__=1"
        "-report"
        "-fxscope"
        "-Wno-xcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"

        "$ENV{XMOS_AIOT_SDK_PATH}/examples/freertos/app_bitstreamless/bitstream_src/${BOARD}/${BOARD}.xn"
        "-DXCOREAI_EXPLORER=1"
        "-lquadspi"
        "-mcmodel=large"
    )

set(FREERTOS_PORT "XCOREAI")

set(APP_SOURCES
    "src/tile_map.xc"

    "src/main.c"
    "src/board_init.c"

    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/dac_init.c"
    "src/example_pipeline/example_pipeline.c"

    "src/intertile_stress_test/intertile_stress_test.c"
    "src/rpc_test/rpc_test.c"
    "src/gpio_test/gpio_test.c"
    #"src/wifi_test/wifi_test.c"
)

set(APP_INCLUDES
    "src"
)

if(DEFINED THIS_XCORE_TILE)
    set(TARGET_NAME "${PROJECT_NAME}_${THIS_XCORE_TILE}.xe")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")

    list(APPEND APP_COMPILER_FLAGS "-DTHIS_XCORE_TILE=${THIS_XCORE_TILE}")
else()
    set(TARGET_NAME "${PROJECT_NAME}.xe")
endif()

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
