
## Create library target
add_library(xcore_sdk_rtos_drivers_qspi_io STATIC)
target_sources(xcore_sdk_rtos_drivers_qspi_io
    PUBLIC
        src/rtos_qspi_flash.c
        src/rtos_qspi_flash_rpc.c
)
target_include_directories(xcore_sdk_rtos_drivers_qspi_io
    PUBLIC
        api
)
target_link_libraries(xcore_sdk_rtos_drivers_qspi_io
    PUBLIC
        sdk::hil::lib_qspi_io
        sdk::rtos::osal
)

## Create an alias
add_library(sdk::rtos::drivers::qspi_io ALIAS xcore_sdk_rtos_drivers_qspi_io)
