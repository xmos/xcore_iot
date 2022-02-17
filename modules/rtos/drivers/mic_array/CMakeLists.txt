
## Create library target
add_library(xcore_sdk_rtos_drivers_mic_array INTERFACE)
target_sources(xcore_sdk_rtos_drivers_mic_array
    INTERFACE
        src/rtos_mic_array.c
        src/rtos_mic_array_rpc.c
)
target_include_directories(xcore_sdk_rtos_drivers_mic_array
    INTERFACE
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_mic_array
    INTERFACE
        sdk::hil::lib_mic_array
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::mic_array ALIAS xcore_sdk_rtos_drivers_mic_array)
