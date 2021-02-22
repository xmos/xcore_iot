cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(USE_XUD_HIL TRUE)
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL FALSE)
set(USE_MIC_ARRAY_HIL TRUE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL TRUE)

set(USE_RTOS_GPIO_DRIVER TRUE)
set(USE_RTOS_I2C_DRIVER FALSE)
set(USE_RTOS_I2S_DRIVER FALSE)
set(USE_RTOS_INTERTILE_DRIVER TRUE)
set(USE_RTOS_MIC_ARRAY_DRIVER TRUE)
set(USE_RTOS_RPC_DRIVER TRUE)
set(USE_RTOS_SPI_DRIVER FALSE)
set(USE_RTOS_USB_DRIVER TRUE)
set(USE_RTOS_SWMEM_DRIVER FALSE)
set(USE_RTOS_QSPI_FLASH_DRIVER TRUE)
set(USE_RTOS_TRACE_DRIVER TRUE)
set(USE_RTOS_WIFI_DRIVER FALSE)

set(USE_TINYUSB TRUE)

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(usb)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(XC CXX C ASM)

## Set app vars
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
    "src/main_0.c"
    "src/main_1.c"
    "src/board_init.c"
    "src/endpoint0.c"
)

set(APP_INCLUDES
    "src"
    "src/usb"
)

add_compile_definitions(
    CFG_TUSB_MCU=OPT_MCU_NONE
    CFG_TUSB_OS=OPT_OS_CUSTOM
    BOARD_DEVICE_RHPORT_NUM=0
    BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED
    CFG_TUSB_ALT_BIT_PACKING_ALIGNMENT=1
    CFG_TUD_EP_MAX=12  ## RTOS_USB_ENDPOINT_COUNT_MAX
    CFG_TUD_TASK_QUEUE_SZ=30

    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    #CFG_TUSB_DEBUG=2

    DEBUG_PRINT_ENABLE=1
    XUD_CORE_CLOCK=700
#    XUD_CORE_CLOCK=500
#    XUD_SERIES_SUPPORT=XUD_X200_SERIES
#    USB_TILE=tile[1]

    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

## Select demo
set(USE_TINYUSB_DEMO_HID_COMPOSITE_TEST          FALSE)
set(USE_TINYUSB_DEMO_AUDIO_TEST                  TRUE)

## In progress
set(USE_TINYUSB_DEMO_CDC_MSC_TEST                FALSE)
set(USE_TINYUSB_DEMO_MSC_DUAL_LUN                FALSE)
set(USE_TINYUSB_DEMO_UAC2_HEADSET                FALSE)

## TODO
set(USE_TINYUSB_DEMO_CDC_DUAL_PORTS_TEST         FALSE)
set(USE_TINYUSB_DEMO_CDC_MSC_FREERTOS_TEST       FALSE)
set(USE_TINYUSB_DEMO_DFU_RUNTIME_TEST            FALSE)
set(USE_TINYUSB_DEMO_DYNAMIC_CONFIGURATION_TEST  FALSE)
set(USE_TINYUSB_DEMO_HID_GENERIC_INOUT_TEST      FALSE)
set(USE_TINYUSB_DEMO_HID_MULTIPLE_INTERFACE_TEST FALSE)
set(USE_TINYUSB_DEMO_MIDI_TEST                   FALSE)
set(USE_TINYUSB_DEMO_NET_LWIP_WEBSERVER          FALSE)
set(USE_TINYUSB_DEMO_USBTMC                      FALSE)
set(USE_TINYUSB_DEMO_WEBUSB_SERIAL               FALSE)

include("${CMAKE_SOURCE_DIR}/src/tinyusb_demos/tinyusb_demos.cmake")

## Create targets
if(DEFINED THIS_XCORE_TILE)
    set(TARGET_NAME "${PROJECT_NAME}_${THIS_XCORE_TILE}.xe")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tile${THIS_XCORE_TILE}")

    add_compile_definitions(THIS_XCORE_TILE=${THIS_XCORE_TILE})
else()
    set(TARGET_NAME "${PROJECT_NAME}.xe")
endif()

add_executable(${TARGET_NAME})

target_sources(
    ${TARGET_NAME}
    PRIVATE
        ${APP_SOURCES}
        ${XMOS_RTOS_PLATFORM_SOURCES}
        ${TINYUSB_DEMO_SOURCES}
    )
target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${APP_INCLUDES}
        ${XMOS_RTOS_PLATFORM_INCLUDES}
        ${TINYUSB_DEMO_INCLUDES}
    )

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
