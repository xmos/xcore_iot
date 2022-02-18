
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_dhcp INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_dhcp
        INTERFACE
            FreeRTOS/dhcpd.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_dhcp
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_dhcp
        INTERFACE
            sdk::rtos::FreeRTOS-Plus-TCP
    )
    target_compile_definitions(xcore_sdk_rtos_sw_services_dhcp
        INTERFACE
            USE_DHCPD=1
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::dhcp ALIAS xcore_sdk_rtos_sw_services_dhcp)
endif()
