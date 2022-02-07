
## Create library target
add_library(xcore_sdk_rtos_drivers_usb STATIC)
target_sources(xcore_sdk_rtos_drivers_usb
    PUBLIC
        src/rtos_usb.c
        src/xud_xfer_data.S
)
set_source_files_properties(src/xud_xfer_data.S PROPERTIES LANGUAGE ASM)
target_include_directories(xcore_sdk_rtos_drivers_usb
    PUBLIC
        api
        src
)
target_link_libraries(xcore_sdk_rtos_drivers_usb
    PUBLIC
        sdk::hil::lib_xud
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::usb ALIAS xcore_sdk_rtos_drivers_usb)
