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

set(LIB_NAME FreeRTOS-Plus-Demos)
set(LIB_VERSION 10.3.0)
set(LIB_ADD_COMPILER_FLAGS "-Os")
set(LIB_XC_SRCS
        ""
    )
set(LIB_C_SRCS
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/FreeRTOS_Plus_FAT_Demos/CreateAndVerifyExampleFiles.c"
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/FreeRTOS_Plus_FAT_Demos/test/ff_stdio_tests_with_cwd.c"

        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Demo_IP_Protocols/Common/FreeRTOS_TCP_server.c"
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Demo_IP_Protocols/FTP/FreeRTOS_FTP_commands.c"
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Demo_IP_Protocols/FTP/FreeRTOS_FTP_server.c"
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/FreeRTOS_Plus_TCP_Demos/TFTPServer.c"

        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Utilities/date_and_time.c"
    )
set(LIB_ASM_SRCS
        ""
    )
set(LIB_INCLUDES
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/FreeRTOS_Plus_TCP_Demos/include"
        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Demo_IP_Protocols/include"

        "FreeRTOS-Plus/FreeRTOS/FreeRTOS-Labs/Demo/Common/Utilities/include"
    )
set(LIB_DEPENDENT_MODULES
        "FreeRTOS-Plus-FAT(>=10.3.0)"
    )
set(LIB_OPTIONAL_HEADERS "")

set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/")

list(TRANSFORM LIB_XC_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_C_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_ASM_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_INCLUDES PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
