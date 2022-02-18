
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create library target
    add_library(xcore_sdk_rtos_osal_freertos INTERFACE)
    target_sources(xcore_sdk_rtos_osal_freertos
        INTERFACE
            FreeRTOS/rtos_osal_event_group_port.c
            FreeRTOS/rtos_osal_heap.c
            FreeRTOS/rtos_osal_mutex_port.c
            FreeRTOS/rtos_osal_queue_port.c
            FreeRTOS/rtos_osal_semaphore_port.c
            FreeRTOS/rtos_osal_thread_port.c
            FreeRTOS/rtos_osal_time.c
    )
    target_include_directories(xcore_sdk_rtos_osal_freertos
        INTERFACE
            api
            FreeRTOS
    )
    target_link_libraries(xcore_sdk_rtos_osal_freertos
        INTERFACE
            sdk::rtos::rtos_support
            sdk::rtos::FreeRTOS_SMP
    )

    ## Create an alias
    add_library(sdk::rtos::osal ALIAS xcore_sdk_rtos_osal_freertos)
endif()
