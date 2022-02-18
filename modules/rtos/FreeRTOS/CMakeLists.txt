if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create FreeRTOS kernel library target
    add_library(xcore_sdk_rtos_freertos_smp INTERFACE)
    target_sources(xcore_sdk_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/port.xc
            FreeRTOS-SMP-Kernel/croutine.c
            FreeRTOS-SMP-Kernel/event_groups.c
            FreeRTOS-SMP-Kernel/list.c
            FreeRTOS-SMP-Kernel/queue.c
            FreeRTOS-SMP-Kernel/stream_buffer.c
            FreeRTOS-SMP-Kernel/tasks.c
            FreeRTOS-SMP-Kernel/timers.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/port.c
            FreeRTOS-SMP-Kernel/portable/MemMang/heap_4.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI/portasm.S
    )
    target_include_directories(xcore_sdk_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/include
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCOREAI
    )
    # Use O2 for tasks.c to give it a slight speedup
    set_source_files_properties(FreeRTOS-SMP-Kernel/tasks.c PROPERTIES COMPILE_FLAGS "-O2")

    ## Create an alias
    add_library(sdk::rtos::FreeRTOS_SMP ALIAS xcore_sdk_rtos_freertos_smp)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A)
    ## Create FreeRTOS kernel library target
    add_library(xcore_sdk_rtos_freertos_smp INTERFACE)
    target_sources(xcore_sdk_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/port.xc
            FreeRTOS-SMP-Kernel/croutine.c
            FreeRTOS-SMP-Kernel/event_groups.c
            FreeRTOS-SMP-Kernel/list.c
            FreeRTOS-SMP-Kernel/queue.c
            FreeRTOS-SMP-Kernel/stream_buffer.c
            FreeRTOS-SMP-Kernel/tasks.c
            FreeRTOS-SMP-Kernel/timers.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/port.c
            FreeRTOS-SMP-Kernel/portable/MemMang/heap_4.c
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200/portasm.S
    )
    target_include_directories(xcore_sdk_rtos_freertos_smp
        INTERFACE
            FreeRTOS-SMP-Kernel/include
            FreeRTOS-SMP-Kernel/portable/ThirdParty/xClang/XCORE200
    )
    # Use O2 for tasks.c to give it a slight speedup
    set_source_files_properties(FreeRTOS-SMP-Kernel/tasks.c PROPERTIES COMPILE_FLAGS "-O2")

    ## Create an alias
    add_library(sdk::rtos::FreeRTOS_SMP ALIAS xcore_sdk_rtos_freertos_smp)
else()
    message(STATUS "FreeRTOS port does not exist for ${CMAKE_SYSTEM_NAME}!  No RTOS targets made.")
endif()



if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create FreeRTOS-Plus-TCP target
    add_library(xcore_sdk_rtos_freertos_plus_tcp INTERFACE)
    target_sources(xcore_sdk_rtos_freertos_plus_tcp
        INTERFACE
            FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
            FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
            FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
            FreeRTOS-Plus-TCP/FreeRTOS_IP.c
            FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
            FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
            FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
            FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
            FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
            FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c
            portable/FreeRTOS-Plus-TCP/NetworkInterface/FreeRTOS_TCP_port.c
    )
    target_include_directories(xcore_sdk_rtos_freertos_plus_tcp
        INTERFACE
            FreeRTOS-Plus-TCP/include
            portable/FreeRTOS-Plus-TCP/Compiler
            portable/FreeRTOS-Plus-TCP/NetworkInterface
    )
    target_link_libraries(xcore_sdk_rtos_freertos_plus_tcp
        INTERFACE
            sdk::rtos::FreeRTOS_SMP
            sdk::lib_random
    )

    ## Create an alias
    add_library(sdk::rtos::FreeRTOS-Plus-TCP ALIAS xcore_sdk_rtos_freertos_plus_tcp)
endif()
