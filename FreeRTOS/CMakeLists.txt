cmake_minimum_required(VERSION 3.13)

if(${USE_FREERTOS_SMP})
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS-SMP" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS")
else()
    add_subdirectory("${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS" "${CMAKE_CURRENT_BINARY_DIR}/FreeRTOS")
endif()
