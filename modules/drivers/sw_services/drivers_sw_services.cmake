cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(DRIVERS_SW_SERVICES_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers/sw_services")

set(WIFI_MANAGER_DIR "${DRIVERS_SW_SERVICES_DIR}/rtos/wifi_manager")

if(NOT DEFINED RTOS_CMAKE_RTOS)
    set(RTOS_CMAKE_RTOS "FreeRTOS") # Only FreeRTOS is currently supported
endif()

if(NOT DEFINED RTOS_WIFI_CHIP)
    set(RTOS_WIFI_CHIP "sl_wf200") # only WiFi module currently supported
endif()

#********************************
# Gather wifi manager sources
#********************************
set(THIS_LIB WIFI_MANAGER)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
)

unset(THIS_LIB)

#**********************
# set user variables
#**********************
set(DRIVERS_SW_SERVICES_SOURCES
    ""
)

set(DRIVERS_SW_SERVICES_INCLUDES
    ""
)

list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_INCLUDES)

set(DRIVERS_SW_SERVICES_NETWORKING_SOURCES
    ${WIFI_MANAGER_SOURCES}
)

set(DRIVERS_SW_SERVICES_NETWORKING_INCLUDES
    ${WIFI_MANAGER_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_INCLUDES)
