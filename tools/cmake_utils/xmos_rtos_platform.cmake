cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED ENV{XMOS_AIOT_SDK_PATH})
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
# Set up for multi-tile builds
#********************************
if(MULTITILE_BUILD)
    set(TARGET_NAME "a.xe")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/${OUTPUT_DIR}/tile${THIS_XCORE_TILE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${OUTPUT_DIR}/tile${THIS_XCORE_TILE}")

    add_compile_definitions(THIS_XCORE_TILE=${THIS_XCORE_TILE})
endif()

#********************************
# Set up hardware target
#********************************
if(NOT DEFINED BOARD)
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
include("${MODULES_DIR}/hil/hil.cmake")
include("${MODULES_DIR}/rtos/rtos.cmake")
include("${MODULES_DIR}/thirdparty/thirdparty.cmake")
include("${MODULES_DIR}/modules.cmake")

#**********************
# set user variables
#**********************
set(XMOS_RTOS_PLATFORM_SOURCES
    ${HIL_SOURCES}
    ${RTOS_SOURCES}
    ${MODULES_SOURCES}
    ${THIRDPARTY_SOURCES}
)

set(XMOS_RTOS_PLATFORM_INCLUDES
    ${HIL_INCLUDES}
    ${RTOS_INCLUDES}
    ${MODULES_INCLUDES}
    ${THIRDPARTY_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_INCLUDES)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES
    ${HIL_SOURCES}
    ${RTOS_SOURCES}
    ${MODULES_SOURCES}
    ${THIRDPARTY_SOURCES}
    ${RTOS_NETWORKING_SOURCES}
)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES
    ${HIL_INCLUDES}
    ${RTOS_INCLUDES}
    ${MODULES_INCLUDES}
    ${THIRDPARTY_INCLUDES}
    ${RTOS_NETWORKING_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES)
