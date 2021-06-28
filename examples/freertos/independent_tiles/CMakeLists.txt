cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_WIFI_MANAGER TRUE)
set(USE_DHCPD TRUE)
set(USE_FATFS TRUE)

## Import XMOS RTOS platform configuration.
## Must be done after setting the configuration options.
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(independent_tiles)
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
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/platform/driver_instances.c"

    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/dac_init.c"
    "src/example_pipeline/example_pipeline.c"

    "src/intertile_stress_test/intertile_stress_test.c"
    "src/rpc_test/rpc_test.c"
    "src/gpio_test/gpio_test.c"
    "src/wifi_test/wifi_test.c"
    "src/swmem_test/swmem_test.c"
)

set(APP_INCLUDES
    "src"
    "src/platform"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    DEBUG_PRINT_ENABLE_IOT_WIFI=1
    SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR|SL_WFX_DEBUG_INIT\)
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
