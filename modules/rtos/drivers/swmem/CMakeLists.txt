
## Create library target
add_library(xcore_sdk_rtos_drivers_swmem STATIC)
target_sources(xcore_sdk_rtos_drivers_swmem
    PUBLIC
        src/rtos_swmem.c
)
target_include_directories(xcore_sdk_rtos_drivers_swmem
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_swmem
    PUBLIC
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::swmem ALIAS xcore_sdk_rtos_drivers_swmem)
