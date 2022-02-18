
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_qspi_io INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_qspi_io
        INTERFACE
            src/rtos_qspi_flash.c
            src/rtos_qspi_flash_rpc.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_qspi_io
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_qspi_io
        INTERFACE
            sdk::hil::lib_qspi_io
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::qspi_io ALIAS xcore_sdk_rtos_drivers_qspi_io)
endif()
