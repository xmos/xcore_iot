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

set(USE_FATFS TRUE)
set(USE_TINYUSB TRUE)
set(USE_DISK_MANAGER_TUSB TRUE)

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

if(${BOARD} STREQUAL "XCORE200-MIC-ARRAY")
    set(SYSCLK_FREQ 500)
else()
    set(SYSCLK_FREQ 700)
endif()

add_compile_definitions(
    CFG_TUSB_MCU=OPT_MCU_NONE
    CFG_TUSB_OS=OPT_OS_CUSTOM
    BOARD_DEVICE_RHPORT_NUM=0
    CFG_TUD_EP_MAX=12  ## RTOS_USB_ENDPOINT_COUNT_MAX
    CFG_TUD_TASK_QUEUE_SZ=8

    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    CFG_TUSB_DEBUG=0

    DEBUG_PRINT_ENABLE=1
    XUD_CORE_CLOCK=${SYSCLK_FREQ}

    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
)

## Select demo
set(XMOS_TINYUSB_DEMO                            FALSE)
set(USE_TINYUSB_DEMO_HID_COMPOSITE_TEST          FALSE)
set(USE_TINYUSB_DEMO_AUDIO_TEST                  TRUE)
set(USE_TINYUSB_DEMO_WEBUSB_SERIAL               FALSE)
set(USE_TINYUSB_DEMO_MIDI_TEST                   FALSE)
set(USE_TINYUSB_DEMO_USBTMC                      FALSE)
set(USE_TINYUSB_DEMO_CDC_MSC_TEST                FALSE)
set(USE_TINYUSB_DEMO_MSC_DUAL_LUN                FALSE)
set(USE_TINYUSB_DEMO_DFU_RUNTIME_TEST            FALSE)
set(USE_TINYUSB_DEMO_CDC_DUAL_PORTS_TEST         FALSE)
set(USE_TINYUSB_DEMO_HID_GENERIC_INOUT_TEST      FALSE)
set(USE_TINYUSB_DEMO_HID_MULTIPLE_INTERFACE_TEST FALSE)

## In progress
set(USE_TINYUSB_DEMO_UAC2_HEADSET                FALSE)

if(XMOS_TINYUSB_DEMO)
    set(DEMO_SOURCES
        "src/xmos_demo/demo_main.c"
        "src/xmos_demo/msc_disk_dual.c"
        "src/xmos_demo/usb_descriptors.c"
    )
    set(DEMO_INCLUDES
        "src/xmos_demo"
    )
    add_compile_definitions(MSC_MAX_DISKS=2)
else()
    include("${CMAKE_SOURCE_DIR}/src/tinyusb_demos/tinyusb_demos.cmake")

    set(DEMO_SOURCES ${TINYUSB_DEMO_SOURCES})
    set(DEMO_INCLUDES ${TINYUSB_DEMO_INCLUDES})

if(USE_TINYUSB_DEMO_USBTMC)
    add_compile_definitions(BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_FULL_SPEED)
else()
    add_compile_definitions(BOARD_DEVICE_RHPORT_SPEED=OPT_MODE_HIGH_SPEED)
endif()
endif()

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
        ${DEMO_SOURCES}
    )
target_include_directories(
    ${TARGET_NAME}
    PRIVATE
        ${APP_INCLUDES}
        ${XMOS_RTOS_PLATFORM_INCLUDES}
        ${DEMO_INCLUDES}
    )

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
