#=============================================================================
# Library creator shall specify the following variables:
#   LIB_NAME                Name of the library
#                           May only contain alphanumeric and space, dash, and
#                           underscore characters.
#                           ex: set(LIB_NAME lib_foo)
#   LIB_VERSION             Version of the library
#                           Must follow semantic version format.
#                           ex: set(LIB_VERSION 1.0.0)
#   LIB_ADD_COMPILER_FLAGS  Additional compiler flags
#                           ex: set(LIB_ADD_COMPILER_FLAGS "-oS")
#   LIB_XC_SRCS             List of all library XC sources
#                           Shall be set to "" if no XC sources exist
#                           ex: set(LIB_XC_SRCS "foo.xc" "bar.xc")
#   LIB_C_SRCS              List of all library C sources
#                           Shall be set to "" if no C sources exist
#                           ex: set(LIB_C_SRCS "foo.c" "bar.c")
#   LIB_ASM_SRCS            List of all library ASM sources
#                           Shall be set to "" if no ASM sources exist
#                           ex: set(LIB_ASM_SRCS "foo.S" "bar.S")
#   LIB_INCLUDES            List of all library include directories
#                           Shall be set to all directories include .h files
#                           ex: set(LIB_INCLUDES "foo/bar")
#   LIB_DEPENDENT_MODULES   List of all dependency libraries with version
#                           Every list item shall include a dependency name,
#                           which must also be the folder in which the library
#                           CMakeLists.txt exists, and a version requirement.
#                           Version requirements shall be in the format:
#                           ([bool req][major].[minor].[patch])
#                           ex: set(LIB_DEPENDENT_MODULES "lib_foo(>=1.0.0)")
#   LIB_OPTIONAL_HEADERS    List of optional header files
#                           Must contain full header file names
#                           ex: set(LIB_OPTIONAL_HEADERS "foo.h" "bar.h")
set(LIB_NAME FreeRTOS)
set(LIB_VERSION 10.3.0)
set(LIB_ADD_COMPILER_FLAGS "-Os")
if(${USE_FREERTOS_SMP})
set(LIB_XC_SRCS
        "portable/XCC/XCORE200/port.xc"
    )
else()
set(LIB_XC_SRCS
        ""
    )
endif()
set(LIB_C_SRCS
        "croutine.c"
        "event_groups.c"
        "list.c"
        "queue.c"
        "stream_buffer.c"
        "tasks.c"
        "timers.c"
        "portable/XCC/XCORE200/port.c"
        "portable/MemMang/heap_4.c"
    )
set(LIB_ASM_SRCS
        "portable/XCC/XCORE200/portasm.S"
    )
set(LIB_INCLUDES
        "include"
        "portable/XCC/XCORE200"
    )
set(LIB_DEPENDENT_MODULES "lib_rtos_support(>=0.5.0)")
set(LIB_OPTIONAL_HEADERS "")

## Temporarily look into FreeRTOS-Plus folder, until switch to kernel submodule
if(${USE_FREERTOS_SMP})
    set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/FreeRTOS-Plus/FreeRTOS-SMP/FreeRTOS/Source/")
else()
    set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/FreeRTOS-Plus/FreeRTOS/FreeRTOS/Source/")
endif()

list(TRANSFORM LIB_XC_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_C_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_ASM_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_INCLUDES PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})

message("${LIB_C_SRCS}")
# #=============================================================================
# # The following part of the CMakeLists includes the common build infrastructure
# # for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
