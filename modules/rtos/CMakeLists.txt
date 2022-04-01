
add_subdirectory(bsp_config)
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
add_subdirectory(drivers/trace)
add_subdirectory(drivers/usb)
add_subdirectory(drivers/wifi)
add_subdirectory(FreeRTOS)
add_subdirectory(osal)
add_subdirectory(rtos_support)
add_subdirectory(sw_services/concurrency_support)
add_subdirectory(sw_services/device_control)
add_subdirectory(sw_services/dhcpd)
add_subdirectory(sw_services/dispatcher)
add_subdirectory(sw_services/fatfs)
add_subdirectory(sw_services/generic_pipeline)
add_subdirectory(sw_services/http)
add_subdirectory(sw_services/inferencing)
add_subdirectory(sw_services/json)
add_subdirectory(sw_services/mqtt)
add_subdirectory(sw_services/sntpd)
add_subdirectory(sw_services/tls_support)
add_subdirectory(sw_services/usb)
add_subdirectory(sw_services/wifi_manager)

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create an aggregate of rtos drivers
    add_library(xcore_sdk_rtos_drivers INTERFACE)
    target_link_libraries(xcore_sdk_rtos_drivers
        INTERFACE
            sdk::rtos::drivers::clock_control
            sdk::rtos::drivers::gpio
            sdk::rtos::drivers::i2c
            sdk::rtos::drivers::intertile
            sdk::rtos::drivers::l2_cache
            sdk::rtos::drivers::qspi_io
            sdk::rtos::drivers::rpc
            sdk::rtos::drivers::spi
            sdk::rtos::drivers::swmem
    )
    add_library(sdk::rtos::drivers ALIAS xcore_sdk_rtos_drivers)

    ## Create an aggregate of rtos audio drivers
    add_library(xcore_sdk_rtos_audio_drivers INTERFACE)
    target_link_libraries(xcore_sdk_rtos_audio_drivers
        INTERFACE
            sdk::rtos::drivers::i2s
            sdk::rtos::drivers::mic_array
        )
    add_library(sdk::rtos::audio_drivers ALIAS xcore_sdk_rtos_audio_drivers)

    ## Create an aggregate of sw_services
    add_library(xcore_sdk_rtos_sw_services INTERFACE)
    target_link_libraries(xcore_sdk_rtos_sw_services
        INTERFACE
            sdk::rtos::sw_services::generic_pipeline
            sdk::rtos::sw_services::concurrency_support
            sdk::rtos::sw_services::dispatcher
            sdk::rtos::sw_services::fatfs
    )
    add_library(sdk::rtos::sw_services ALIAS xcore_sdk_rtos_sw_services)

    # Create an aggregate of rtos libraries
    add_library(xcore_sdk_rtos_freertos INTERFACE)
    target_link_libraries(xcore_sdk_rtos_freertos
        INTERFACE
            sdk::rtos::drivers
            sdk::rtos::osal
            sdk::rtos::rtos_support
            sdk::rtos::sw_services
            sdk::rtos::FreeRTOS_SMP
    )
    add_library(sdk::rtos_freertos ALIAS xcore_sdk_rtos_freertos)

    # Create an aggregate of usb device control libraries
    add_library(xcore_sdk_rtos_usb_device_control INTERFACE)
    target_link_libraries(xcore_sdk_rtos_usb_device_control
        INTERFACE
            sdk::rtos::sw_services::device_control
            sdk::rtos::sw_services::usb
    )
    add_library(sdk::rtos::usb_device_control ALIAS xcore_sdk_rtos_usb_device_control)

    # Create an aggregate of wifi libraries
    add_library(xcore_sdk_rtos_wifi INTERFACE)
    target_link_libraries(xcore_sdk_rtos_wifi
        INTERFACE
            sdk::rtos::drivers::wifi
            sdk::rtos::FreeRTOS-Plus-TCP
            sdk::rtos::sw_services
            sdk::rtos::sw_services::dhcp
            sdk::rtos::sw_services::wifi_manager
    )
    add_library(sdk::rtos::wifi ALIAS xcore_sdk_rtos_wifi)
endif()
