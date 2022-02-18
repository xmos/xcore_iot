
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_trace INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_trace
        INTERFACE
            ASCII/ascii_trace.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_trace
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_trace
        INTERFACE
            sdk::rtos::FreeRTOS_SMP
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::trace ALIAS xcore_sdk_rtos_drivers_trace)
endif()
