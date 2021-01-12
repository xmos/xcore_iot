cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(DRIVERS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers")

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
include("${DRIVERS_DIR}/hil/drivers_hil.cmake")
include("${DRIVERS_DIR}/rtos/drivers_rtos.cmake")
include("${DRIVERS_DIR}/sw_services/drivers_sw_services.cmake")

#**********************
# Set user variables
#**********************
set(DRIVERS_SOURCES
    ${DRIVERS_HIL_SOURCES}
    ${DRIVERS_RTOS_SOURCES}
    ${DRIVERS_SW_SERVICES_SOURCES}
)

set(DRIVERS_INCLUDES
    ${DRIVERS_HIL_INCLUDES}
    ${DRIVERS_RTOS_INCLUDES}
    ${DRIVERS_SW_SERVICES_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_INCLUDES)

set(DRIVERS_NETWORKING_SOURCES
    ${DRIVERS_RTOS_NETWORKING_SOURCES}
    ${DRIVERS_SW_SERVICES_NETWORKING_SOURCES}
)

set(DRIVERS_NETWORKING_INCLUDES
    ${DRIVERS_RTOS_NETWORKING_INCLUDES}
    ${DRIVERS_SW_SERVICES_NETWORKING_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_NETWORKING_INCLUDES)
