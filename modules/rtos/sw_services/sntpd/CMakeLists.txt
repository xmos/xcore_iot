
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_sntpd INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_sntpd
        INTERFACE
            FreeRTOS/sntpd.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_sntpd
        INTERFACE
            FreeRTOS
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_sntpd
        INTERFACE
            sdk::rtos::FreeRTOS-Plus-TCP
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::sntpd ALIAS xcore_sdk_rtos_sw_services_sntpd)
endif()
