
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_sw_services_fatfs INTERFACE)
    target_sources(xcore_sdk_rtos_sw_services_fatfs
        INTERFACE
            FreeRTOS/diskio.c
            FreeRTOS/ffsystem.c
            FreeRTOS/fs_support.c
            thirdparty/src/ff.c
            thirdparty/src/ffunicode.c
    )
    target_include_directories(xcore_sdk_rtos_sw_services_fatfs
        INTERFACE
            FreeRTOS
            thirdparty/api
    )
    target_link_libraries(xcore_sdk_rtos_sw_services_fatfs
        INTERFACE
            sdk::rtos::osal
            sdk::rtos::drivers::qspi_io
            sdk::rtos::FreeRTOS_SMP
    )
    target_compile_definitions(xcore_sdk_rtos_sw_services_fatfs
        INTERFACE
            USE_FATFS=1
    )

    ## Create an alias
    add_library(sdk::rtos::sw_services::fatfs ALIAS xcore_sdk_rtos_sw_services_fatfs)
else()
    add_subdirectory(host)
endif()
