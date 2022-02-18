
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_gpio INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_gpio
        INTERFACE
            src/rtos_gpio.c
            src/rtos_gpio_rpc.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_gpio
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_gpio
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::gpio ALIAS xcore_sdk_rtos_drivers_gpio)
endif()
