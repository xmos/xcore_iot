cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(RTOS_SUPPORT_DIR "${XCORE_SDK_PATH}/modules/rtos/rtos_support")

#********************************
# Gather RTOS support sources
#********************************
set(THIS_LIB RTOS_SUPPORT)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${RTOS_SUPPORT_DIR}/src/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${RTOS_SUPPORT_DIR}/api"
    "${RTOS_SUPPORT_DIR}/src"
)

message("${COLOR_GREEN}Gathering ${THIS_LIB}...${COLOR_RESET}")
unset(THIS_LIB)

list(REMOVE_DUPLICATES RTOS_SUPPORT_SOURCES)
list(REMOVE_DUPLICATES RTOS_SUPPORT_INCLUDES)
