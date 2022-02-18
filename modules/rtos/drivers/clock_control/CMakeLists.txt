
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_clock_control INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_clock_control
        INTERFACE
            src/rtos_clock_control.c
            src/rtos_clock_control_rpc.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_clock_control
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_clock_control
        INTERFACE
            sdk::hil::lib_clock_control
            sdk::rtos::osal
            sdk::rtos::sw_services::concurrency_support
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::clock_control ALIAS xcore_sdk_rtos_drivers_clock_control)
endif()
