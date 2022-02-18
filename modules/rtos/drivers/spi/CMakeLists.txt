
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_spi INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_spi
        INTERFACE
            src/rtos_spi_master.c
            src/rtos_spi_master_rpc.c
            src/rtos_spi_slave.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_spi
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_spi
        INTERFACE
            sdk::hil::lib_spi
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::spi ALIAS xcore_sdk_rtos_drivers_spi)
endif()
