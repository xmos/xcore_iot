cmake_minimum_required(VERSION 3.14)

IF(NOT DEFINED ENV{XMOS_AIOT_SDK_PATH})
    message(FATAL_ERROR "Environment var XMOS_AIOT_SDK_PATH must be set before including xmos_utils.cmake")
endif()

# Set up compiler
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#**********************
# Paths
#**********************
set(MODULES_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules")

#********************************
# Gather various sources
#********************************
include("${MODULES_DIR}/drivers/drivers.cmake")
include("$ENV{XMOS_AIOT_SDK_PATH}/modules/modules.cmake")

#**********************
# set user variables
#**********************
set(XMOS_RTOS_PLATFORM_SOURCES
    ${DRIVERS_SOURCES}
    ${MODULES_SOURCES}
)

set(XMOS_RTOS_PLATFORM_INCLUDES
    ${DRIVERS_INCLUDES}
    ${MODULES_INCLUDES}
    ${MODULES_DIR}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_INCLUDES)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES
    ${DRIVERS_SOURCES}
    ${MODULES_SOURCES}
    ${DRIVERS_NETWORKING_SOURCES}
)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES
    ${DRIVERS_INCLUDES}
    ${MODULES_INCLUDES}
    ${DRIVERS_NETWORKING_INCLUDES}
    ${MODULES_DIR}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES)
