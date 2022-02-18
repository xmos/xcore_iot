
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_wifi_manager INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_wifi_manager
        INTERFACE
            FreeRTOS/wifi_connection_mgr.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_wifi_manager
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_wifi_manager
        INTERFACE
            sdk::rtos::FreeRTOS-Plus-TCP
            sdk::rtos::sw_services::dhcp
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::wifi_manager ALIAS xcore_sdk_rtos_sw_services_wifi_manager)
endif()
