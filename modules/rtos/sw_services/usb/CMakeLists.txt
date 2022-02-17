file(GLOB         TUSB_ROOT_SOURCES   thirdparty/tinyusb_src/src/*.c)
file(GLOB_RECURSE TUSB_CLASS_SOURCES  thirdparty/tinyusb_src/src/class/*/*.c)
file(GLOB_RECURSE TUSB_COMMON_SOURCES thirdparty/tinyusb_src/src/common/*.c)
file(GLOB_RECURSE TUSB_DEVICE_SOURCES thirdparty/tinyusb_src/src/device/*.c)

## Create library target
add_library(xcore_sdk_rtos_sw_services_usb INTERFACE)
target_sources(xcore_sdk_rtos_sw_services_usb
    INTERFACE
        portable/dcd_xcore.c
        FreeRTOS/usb_support.c
        ${TUSB_ROOT_SOURCES}
        ${TUSB_CLASS_SOURCES}
        ${TUSB_COMMON_SOURCES}
        ${TUSB_DEVICE_SOURCES}
)
target_include_directories(xcore_sdk_rtos_sw_services_usb
    INTERFACE
        api
        portable
        thirdparty/tinyusb_src/src
)
target_link_libraries(xcore_sdk_rtos_sw_services_usb
    INTERFACE
        sdk::rtos::osal
        sdk::rtos::drivers::usb
        sdk::rtos::FreeRTOS_SMP
)

## Create an alias
add_library(sdk::rtos::sw_services::usb ALIAS xcore_sdk_rtos_sw_services_usb)
