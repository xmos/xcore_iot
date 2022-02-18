
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_rpc INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_rpc
        INTERFACE
            src/rtos_rpc.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_rpc
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_rpc
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::rpc ALIAS xcore_sdk_rtos_drivers_rpc)
endif()
