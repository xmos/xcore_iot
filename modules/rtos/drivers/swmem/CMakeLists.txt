
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(xcore_sdk_rtos_drivers_swmem INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_swmem
        INTERFACE
            src/rtos_swmem.c
    )
    target_include_directories(xcore_sdk_rtos_drivers_swmem
        INTERFACE
            api
    )
    target_link_libraries(xcore_sdk_rtos_drivers_swmem
        INTERFACE
            sdk::rtos::osal
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::swmem ALIAS xcore_sdk_rtos_drivers_swmem)
endif()
