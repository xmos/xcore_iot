
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_intertile INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_intertile
        INTERFACE
            src/rtos_intertile.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_intertile
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_intertile
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::intertile ALIAS xcore_sdk_rtos_drivers_intertile)
endif()
