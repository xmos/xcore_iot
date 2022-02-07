
## Create library target
add_library(xcore_sdk_rtos_sw_services_concurrency_support STATIC)
target_sources(xcore_sdk_rtos_sw_services_concurrency_support
    PUBLIC
        src/mrsw_lock.c
)
target_include_directories(xcore_sdk_rtos_sw_services_concurrency_support
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_sw_services_concurrency_support
    PUBLIC
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::concurrency_support ALIAS xcore_sdk_rtos_sw_services_concurrency_support)
