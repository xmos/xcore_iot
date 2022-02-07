
## Create library target
add_library(xcore_sdk_rtos_drivers_clock_control STATIC)
target_sources(xcore_sdk_rtos_drivers_clock_control
    PUBLIC
        src/rtos_clock_control.c
        src/rtos_clock_control_rpc.c
)
target_include_directories(xcore_sdk_rtos_drivers_clock_control
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_clock_control
    PUBLIC
        sdk::hil::lib_clock_control
        sdk::rtos::osal
        sdk::rtos::sw_services::concurrency_support
)

## Create an alias
add_library(sdk::rtos::drivers::clock_control ALIAS xcore_sdk_rtos_drivers_clock_control)
