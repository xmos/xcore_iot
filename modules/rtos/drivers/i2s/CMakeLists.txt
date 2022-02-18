
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_i2s INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_i2s
        INTERFACE
            src/rtos_i2s.c
            src/rtos_i2s_rpc.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_i2s
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_i2s
        INTERFACE
            sdk::hil::lib_i2s
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::i2s ALIAS xcore_sdk_rtos_drivers_i2s)
endif()
