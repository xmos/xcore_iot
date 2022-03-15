
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_generic_pipeline INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_generic_pipeline
        INTERFACE
            src/generic_pipeline.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_generic_pipeline
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_generic_pipeline
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::generic_pipeline ALIAS xcore_sdk_rtos_sw_services_generic_pipeline)
endif()
