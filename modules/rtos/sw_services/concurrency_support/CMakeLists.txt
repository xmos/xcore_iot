
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_concurrency_support INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_concurrency_support
        INTERFACE
            src/mrsw_lock.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_concurrency_support
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_concurrency_support
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::concurrency_support ALIAS xcore_sdk_rtos_sw_services_concurrency_support)
endif()
