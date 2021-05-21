cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(RTOS_DIR "$ENV{XCORE_SDK_PATH}/modules/rtos")
set(OSAL_DIR "${RTOS_DIR}/osal")

#**********************
# Set default configuration variables
#**********************
if(NOT DEFINED RTOS_CMAKE_RTOS)
    set(RTOS_CMAKE_RTOS "FreeRTOS") # Only FreeRTOS is currently supported
endif()

if(NOT DEFINED RTOS_WIFI_CHIP)
    set(RTOS_WIFI_CHIP "sl_wf200") # only WiFi module currently supported
endif()

#********************************
# Gather various sources
#********************************
include("${RTOS_DIR}/drivers/drivers.cmake")
include("${RTOS_DIR}/${RTOS_CMAKE_RTOS}/kernel.cmake")
include("${RTOS_DIR}/rtos_support/rtos_support.cmake")
include("${RTOS_DIR}/sw_services/sw_services.cmake")

#********************************
# Gather OSAL sources
#********************************
set(THIS_LIB OSAL)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}"
)

message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
unset(THIS_LIB)

#**********************
# Set user variables
#**********************
set(RTOS_SOURCES
    ${OSAL_SOURCES}
    ${DRIVERS_RTOS_SOURCES}
    ${RTOS_SUPPORT_SOURCES}
    ${KERNEL_SOURCES}
    ${SW_SERVICES_SOURCES}
)

set(RTOS_INCLUDES
    ${OSAL_INCLUDES}
    ${DRIVERS_RTOS_INCLUDES}
    ${RTOS_SUPPORT_INCLUDES}
    ${KERNEL_INCLUDES}
    ${SW_SERVICES_INCLUDES}
)

list(REMOVE_DUPLICATES RTOS_SOURCES)
list(REMOVE_DUPLICATES RTOS_INCLUDES)

set(RTOS_NETWORKING_SOURCES
    ${DRIVERS_RTOS_NETWORKING_SOURCES}
    ${KERNEL_NETWORKING_SOURCES}
    ${SW_SERVICES_NETWORKING_SOURCES}
)

set(RTOS_NETWORKING_INCLUDES
    ${DRIVERS_RTOS_NETWORKING_INCLUDES}
    ${KERNEL_NETWORKING_INCLUDES}
    ${SW_SERVICES_NETWORKING_INCLUDES}
)

list(REMOVE_DUPLICATES RTOS_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES RTOS_NETWORKING_INCLUDES)
