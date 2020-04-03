cmake_minimum_required(VERSION 3.13)

if(${USE_FREERTOS_SMP})
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS-SMP/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS-Plus-TCP")
else()
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS-Plus-TCP")

endif()
