cmake_minimum_required(VERSION 3.13)

if(${USE_FREERTOS_SMP})
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS-SMP/FreeRTOS-Labs/Source/FreeRTOS-Plus-FAT" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS-Plus-FAT")
else()
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/FreeRTOS-Labs/Source/FreeRTOS-Plus-FAT" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS-Plus-FAT")

endif()
