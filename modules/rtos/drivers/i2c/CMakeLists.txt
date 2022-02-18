
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_i2c INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_i2c
        INTERFACE
            src/rtos_i2c_master.c
            src/rtos_i2c_master_rpc.c
            src/rtos_i2c_slave.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_i2c
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_i2c
        INTERFACE
            sdk::hil::lib_i2c
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::i2c ALIAS xcore_sdk_rtos_drivers_i2c)
endif()
