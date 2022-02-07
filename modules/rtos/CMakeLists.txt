
add_subdirectory(drivers/clock_control)
add_subdirectory(drivers/gpio)
add_subdirectory(drivers/i2c)
add_subdirectory(drivers/i2s)
add_subdirectory(drivers/intertile)
add_subdirectory(drivers/l2_cache)
add_subdirectory(drivers/mic_array)
add_subdirectory(drivers/qspi_flash)
add_subdirectory(drivers/rpc)
add_subdirectory(drivers/spi)
add_subdirectory(drivers/swmem)
# add_subdirectory(drivers/trace)
add_subdirectory(drivers/usb)
add_subdirectory(drivers/wifi)
add_subdirectory(FreeRTOS)
add_subdirectory(osal)
add_subdirectory(rtos_support)
add_subdirectory(sw_services/concurrency_support)
# add_subdirectory(sw_services/device_control)
# add_subdirectory(sw_services/dhcpd)
# add_subdirectory(sw_services/dispatcher)
add_subdirectory(sw_services/fatfs)
# add_subdirectory(sw_services/http)
# add_subdirectory(sw_services/json)
# add_subdirectory(sw_services/mqtt)
# add_subdirectory(sw_services/sntpd)
# add_subdirectory(sw_services/tls_support)
add_subdirectory(sw_services/usb)
# add_subdirectory(sw_services/wifi_manager)

## Create an aggregate of rtos drivers
add_library(xcore_sdk_rtos_drivers INTERFACE)
target_link_libraries(xcore_sdk_rtos_drivers
    INTERFACE
        sdk::rtos::drivers::clock_control
        sdk::rtos::drivers::gpio
        sdk::rtos::drivers::i2c
        sdk::rtos::drivers::i2s
        sdk::rtos::drivers::intertile
        sdk::rtos::drivers::l2_cache
        sdk::rtos::drivers::mic_array
        sdk::rtos::drivers::qspi_io
        sdk::rtos::drivers::rpc
        sdk::rtos::drivers::spi
        sdk::rtos::drivers::swmem
        sdk::rtos::drivers::usb
        sdk::rtos::drivers::wifi
)
add_library(sdk::rtos::drivers ALIAS xcore_sdk_rtos_drivers)

## Create an aggregate of sw_services
add_library(xcore_sdk_rtos_sw_services INTERFACE)
target_link_libraries(xcore_sdk_rtos_sw_services
    INTERFACE
        sdk::rtos::sw_services::concurrency_support
        # sdk::rtos::sw_services::device_control
        # sdk::rtos::sw_services::dhcpd
        # sdk::rtos::sw_services::dispatcher
        sdk::rtos::sw_services::fatfs
        # sdk::rtos::sw_services::http
        # sdk::rtos::sw_services::json
        # sdk::rtos::sw_services::mqtt
        # sdk::rtos::sw_services::sntpd
        # sdk::rtos::sw_services::tls_support
        sdk::rtos::sw_services::usb
        # sdk::rtos::sw_services::wifi_manager
)
add_library(sdk::rtos::sw_services ALIAS xcore_sdk_rtos_sw_services)

## Create an aggregate of rtos libraries
add_library(xcore_sdk_rtos_freertos INTERFACE)
target_link_libraries(xcore_sdk_rtos_freertos
    INTERFACE
        sdk::rtos::drivers
        sdk::rtos::osal
        sdk::rtos::rtos_support
        sdk::rtos::sw_services
        sdk::rtos::osal_freertos
        sdk::rtos::FreeRTOS_SMP
        sdk::rtos::FreeRTOS-Plus-TCP
)
add_library(sdk::rtos_freertos ALIAS xcore_sdk_rtos_freertos)
