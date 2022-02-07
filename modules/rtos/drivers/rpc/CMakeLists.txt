
## Create library target
add_library(xcore_sdk_rtos_drivers_rpc STATIC)
target_sources(xcore_sdk_rtos_drivers_rpc
    PUBLIC
        src/rtos_rpc.c
)
target_include_directories(xcore_sdk_rtos_drivers_rpc
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_rpc
    PUBLIC
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::rpc ALIAS xcore_sdk_rtos_drivers_rpc)
