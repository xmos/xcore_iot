
## Create library target
add_library(xcore_sdk_rtos_drivers_gpio STATIC)
target_sources(xcore_sdk_rtos_drivers_gpio
    PUBLIC
        src/rtos_gpio.c
        src/rtos_gpio_rpc.c
)
target_include_directories(xcore_sdk_rtos_drivers_gpio
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_gpio
    PUBLIC
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::gpio ALIAS xcore_sdk_rtos_drivers_gpio)
