
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(xcore_sdk_device_memory_support_bare_metal STATIC)
    target_sources(xcore_sdk_device_memory_support_bare_metal
        PRIVATE
            bare-metal/xcore_device_memory.c
    )
    target_include_directories(xcore_sdk_device_memory_support_bare_metal
        PUBLIC
            bare-metal
    )

    ## Create an alias
    add_library(sdk::device_memory_support_bare_metal ALIAS xcore_sdk_device_memory_support_bare_metal)


    ## Create library target
    add_library(xcore_sdk_device_memory_support_rtos INTERFACE)
    target_sources(xcore_sdk_device_memory_support_rtos
        INTERFACE
            rtos/xcore_device_memory.c
    )
    target_include_directories(xcore_sdk_device_memory_support_rtos
        INTERFACE
            rtos
    )
    target_link_libraries(xcore_sdk_device_memory_support_rtos
        INTERFACE
            sdk::rtos::drivers::qspi_io
            sdk::rtos::drivers::swmem
    )

    ## Create an alias
    add_library(sdk::device_memory_support_rtos ALIAS xcore_sdk_device_memory_support_rtos)
endif()
