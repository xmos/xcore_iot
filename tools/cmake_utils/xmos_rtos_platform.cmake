cmake_minimum_required(VERSION 3.14)

IF(NOT DEFINED ENV{XMOS_AIOT_SDK_PATH})
    message(FATAL_ERROR "Environment var XMOS_AIOT_SDK_PATH must be set before including xmos_utils.cmake")
endif()

#**********************
# Set up some custom vars
#**********************
string(ASCII 27 ESC)
set(COLOR_RESET   "${ESC}[m")
set(COLOR_RED     "${ESC}[31m")
set(COLOR_GREEN   "${ESC}[32m")
set(COLOR_YELLOW  "${ESC}[33m")
set(COLOR_BLUE    "${ESC}[34m")
set(COLOR_MAGENTA "${ESC}[35m")
set(COLOR_CYAN    "${ESC}[36m")
set(COLOR_WHITE   "${ESC}[37m")

#********************************
# Set up compiler
#********************************
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#********************************
# Set up hardware target
#********************************
IF(NOT DEFINED BOARD)
    message(FATAL_ERROR "BOARD must be defined to specify the hardware target.")
endif()
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/board_support/${BOARD}.cmake")

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
    ${DRIVERS_NETWORKING_SOURCES}
)

set(XMOS_RTOS_PLATFORM_INCLUDES
    ${DRIVERS_INCLUDES}
    ${MODULES_INCLUDES}
    ${MODULES_DIR}
    ${DRIVERS_NETWORKING_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_INCLUDES)
