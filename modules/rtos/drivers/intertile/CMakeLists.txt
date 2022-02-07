
## Create library target
add_library(xcore_sdk_rtos_drivers_intertile STATIC)
target_sources(xcore_sdk_rtos_drivers_intertile
    PUBLIC
        src/rtos_intertile.c
)
target_include_directories(xcore_sdk_rtos_drivers_intertile
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_intertile
    PUBLIC
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::intertile ALIAS xcore_sdk_rtos_drivers_intertile)
