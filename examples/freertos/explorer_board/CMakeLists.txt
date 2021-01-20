cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_WIFI_MANAGER TRUE)
set(USE_DHCPD TRUE)
set(USE_FATFS TRUE)
# set(USE_HTTP_PARSER TRUE)
# set(USE_JSON_PARSER TRUE)
# set(USE_MQTT TRUE)
# set(USE_SNTPD TRUE)
# set(USE_TLS_SUPPORT TRUE)

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(explorer_board)
set(PROJECT_VERSION 0.6.0)

## Enable languages for project
enable_language(XC CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-lquadspi"
    "-mcmodel=large"
    "-Wxcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/bitstream_src/${BOARD}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/main_0.c"
    "src/main_1.c"
    "src/board_init.c"

    # "src/audio_hw_config/audio_hw_config.c"
    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/example_pipeline.c"
    "src/example_pipeline/dac_init.c"
    "src/gpio_ctrl/gpio_ctrl.c"
    "src/network_demos/network_setup.c"
    # "src/NetworkManager/network.c"
    # "src/queue_to_i2s/queue_to_i2s.c"
    # "src/queue_to_tcp_stream/queue_to_tcp_stream.c"
    # "src/tcp_stream_to_queue/tcp_stream_to_queue.c"
    # "src/thruput_test/thruput_test.c"
    # "src/udp_cli/CLI-commands.c"
    # "src/udp_cli/FreeRTOS_CLI.c"
    # "src/udp_cli/UDPCommandServer.c"
    # "src/tls_echo_demo/tls_echo_demo.c"
    # "src/tls_echo_server/tls_echo_server.c"
    # "src/http_demo/http_demo.c"
    # "src/mqtt_demo/mqtt_demo_client.c"
    "src/mem_analysis/mem_analysis.c"
)

set(APP_INCLUDES
    "src"
    # "src/audio_hw_config"
    # "src/audio_pipeline"
    "src/gpio_ctrl"
    "src/network_demos"
    # "src/NetworkManager"
    # "src/queue_to_i2s"
    # "src/queue_to_tcp_stream"
    # "src/tcp_stream_to_queue"
    # "src/thruput_test"
    # "src/udp_cli"
    # "src/tls_echo_demo"
    # "src/tls_echo_server"
    # "src/http_demo"
    # "src/mqtt_demo"
    "src/mem_analysis"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    DEBUG_PRINT_ENABLE_IOT_WIFI=1
    DEBUG_PRINT_ENABLE_WIFI_CONN_MGR=1
    MIC_DUAL_ENABLED=1
    # MQTT_TASK=1
    DEBUG_PRINT_ENABLE_LIB_SOC_SW_WIFI=1
    SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR|SL_WFX_DEBUG_INIT\)
    # MQTTCLIENT_PLATFORM_HEADER=MQTTFreeRTOS.h
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

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
