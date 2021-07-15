cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(FREERTOS_DIR "$ENV{XCORE_SDK_PATH}/modules/rtos/FreeRTOS/FreeRTOS-Kernel")
set(FREERTOS_SMP_DIR "$ENV{XCORE_SDK_PATH}/modules/rtos/FreeRTOS/FreeRTOS-SMP-Kernel")

#********************************
# Gather FreeRTOS sources
#********************************
set(THIS_LIB FREERTOS)

if(NOT DEFINED FREERTOS_PORT)
    set(FREERTOS_PORT "XCOREAI")
    message("${COLOR_CYAN}FreeRTOS port set to XCOREAI by default${COLOR_RESET}")
endif()

if(FREERTOS_PORT STREQUAL "XCOREAI")
    set(${THIS_LIB}_FLAGS "-Os -march=xs3a")
elseif(FREERTOS_PORT STREQUAL "XCORE200")
    set(${THIS_LIB}_FLAGS "-Os -march=xs2a")
endif()

if(NOT DEFINED FREERTOS_SMP)
    set(FREERTOS_SMP True)
    message("${COLOR_CYAN}FreeRTOS SMP used by default.  Set FREERTOS_SMP to False to use single core kernel.${COLOR_RESET}")
endif()

if(FREERTOS_SMP)
    set(${THIS_LIB}_DIR ${FREERTOS_SMP_DIR})
else()
    set(${THIS_LIB}_DIR ${FREERTOS_DIR})
endif()

set(${THIS_LIB}_SOURCES
    "${${THIS_LIB}_DIR}/portable/ThirdParty/xClang/${FREERTOS_PORT}/port.xc"
    "${${THIS_LIB}_DIR}/croutine.c"
    "${${THIS_LIB}_DIR}/event_groups.c"
    "${${THIS_LIB}_DIR}/list.c"
    "${${THIS_LIB}_DIR}/queue.c"
    "${${THIS_LIB}_DIR}/stream_buffer.c"
    "${${THIS_LIB}_DIR}/tasks.c"
    "${${THIS_LIB}_DIR}/timers.c"
    "${${THIS_LIB}_DIR}/portable/ThirdParty/xClang/${FREERTOS_PORT}/port.c"
    "${${THIS_LIB}_DIR}/portable/MemMang/heap_4.c"
    "${${THIS_LIB}_DIR}/portable/ThirdParty/xClang/${FREERTOS_PORT}/portasm.S"
)

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
# Use O2 for tasks.c to give it a slight speedup
set_source_files_properties(${${THIS_LIB}_DIR}/tasks.c PROPERTIES COMPILE_FLAGS "${${THIS_LIB}_FLAGS} -O2")

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/include"
    "${${THIS_LIB}_DIR}/portable/ThirdParty/xClang/${FREERTOS_PORT}"
)

add_compile_definitions(RTOS_FREERTOS)

message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
unset(THIS_LIB)

#********************************
# Gather FreeRTOS-Plus-TCP sources
#********************************
set(THIS_LIB FREERTOS_PLUS_TCP)
set(${THIS_LIB}_FLAGS "-Os")

# Always use the sources from the single core kernel dir for Plus TCP
set(${THIS_LIB}_DIR "$ENV{XCORE_SDK_PATH}/modules/rtos/FreeRTOS/FreeRTOS-Plus-TCP")
set(${THIS_LIB}_PORTABLE_DIR "$ENV{XCORE_SDK_PATH}/modules/rtos/FreeRTOS/portable/FreeRTOS-Plus-TCP")

set(${THIS_LIB}_SOURCES
    "${${THIS_LIB}_DIR}/FreeRTOS_ARP.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_DHCP.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_DNS.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_IP.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_Sockets.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_Stream_Buffer.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_TCP_IP.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_TCP_WIN.c"
    "${${THIS_LIB}_DIR}/FreeRTOS_UDP_IP.c"
    "${${THIS_LIB}_DIR}/portable/BufferManagement/BufferAllocation_2.c"
    "${${THIS_LIB}_PORTABLE_DIR}/NetworkInterface/FreeRTOS_TCP_port.c"
)

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/include"
    "${${THIS_LIB}_PORTABLE_DIR}/Compiler"
    "${${THIS_LIB}_PORTABLE_DIR}/NetworkInterface"
)

message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
unset(THIS_LIB)

#**********************
# Set user variables
#**********************
set(KERNEL_SOURCES
    ${FREERTOS_SOURCES}
)

set(KERNEL_INCLUDES
    ${FREERTOS_INCLUDES}
)

list(REMOVE_DUPLICATES KERNEL_SOURCES)
list(REMOVE_DUPLICATES KERNEL_INCLUDES)

set(KERNEL_NETWORKING_SOURCES
    ${FREERTOS_PLUS_TCP_SOURCES}
)

set(KERNEL_NETWORKING_INCLUDES
    ${FREERTOS_PLUS_TCP_INCLUDES}
)

list(REMOVE_DUPLICATES KERNEL_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES KERNEL_NETWORKING_INCLUDES)
