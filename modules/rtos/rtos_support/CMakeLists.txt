
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_rtos_support INTERFACE)
    target_sources(xcore_sdk_rtos_rtos_support
        INTERFACE
            src/rtos_cores.c
            src/rtos_irq.c
            src/rtos_locks.c
            src/rtos_time.c
    )
    target_include_directories(xcore_sdk_rtos_rtos_support
        INTERFACE
            api
            src
    )
    target_link_libraries(xcore_sdk_rtos_rtos_support
        INTERFACE
            sdk::rtos::FreeRTOS_SMP ## TEMP WORKAROUND
    )

    ## Create an alias
    add_library(sdk::rtos::rtos_support ALIAS xcore_sdk_rtos_rtos_support)
endif()
