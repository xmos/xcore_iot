
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_l2_cache INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_l2_cache
        INTERFACE
            src/rtos_l2_cache.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_l2_cache
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_l2_cache
        INTERFACE
            sdk::hil::lib_l2_cache
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::l2_cache ALIAS xcore_sdk_rtos_drivers_l2_cache)
endif()
