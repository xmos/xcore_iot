
## Create library target
add_library(xcore_sdk_rtos_drivers_spi STATIC)
target_sources(xcore_sdk_rtos_drivers_spi
    PUBLIC
        src/rtos_spi_master.c
        src/rtos_spi_master_rpc.c
        src/rtos_spi_slave.c
)
target_include_directories(xcore_sdk_rtos_drivers_spi
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_spi
    PUBLIC
        sdk::hil::lib_spi
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::spi ALIAS xcore_sdk_rtos_drivers_spi)
