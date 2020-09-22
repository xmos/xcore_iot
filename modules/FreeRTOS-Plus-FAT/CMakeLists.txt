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

set(LIB_NAME FreeRTOS-Plus-FAT)
set(LIB_VERSION 10.3.0)
set(LIB_ADD_COMPILER_FLAGS "-Os")
set(LIB_XC_SRCS
        ""
    )
set(LIB_CXX_SRCS
        ""
    )
set(LIB_C_SRCS
        "Lab-Project-FreeRTOS-FAT/ff_crc.c"
        #"ff_dev_support.c"
        "Lab-Project-FreeRTOS-FAT/ff_dir.c"
        "Lab-Project-FreeRTOS-FAT/ff_error.c"
        "Lab-Project-FreeRTOS-FAT/ff_fat.c"
        "Lab-Project-FreeRTOS-FAT/ff_file.c"
        "Lab-Project-FreeRTOS-FAT/ff_format.c"
        "Lab-Project-FreeRTOS-FAT/ff_ioman.c"
        "Lab-Project-FreeRTOS-FAT/ff_locking.c"
        "Lab-Project-FreeRTOS-FAT/ff_memory.c"
        "Lab-Project-FreeRTOS-FAT/ff_stdio.c"
        "Lab-Project-FreeRTOS-FAT/ff_string.c"
        "Lab-Project-FreeRTOS-FAT/ff_sys.c"
        "Lab-Project-FreeRTOS-FAT/ff_time.c"
        "Lab-Project-FreeRTOS-FAT/portable/common/ff_ramdisk.c"
        "portable/XCC/XCORE200/FreeRTOS-Plus-FAT/ff_flashdisk.c"
    )
set(LIB_ASM_SRCS
        ""
    )
set(LIB_INCLUDES
        "Lab-Project-FreeRTOS-FAT/include"
        "Lab-Project-FreeRTOS-FAT/portable/common"
        "portable/XCC/XCORE200/FreeRTOS-Plus-FAT"
    )
set(LIB_DEPENDENT_MODULES
        "FreeRTOS(>=10.3.0)"
        "lib_soc(>=0.5.0)"
    )
set(LIB_OPTIONAL_HEADERS "")

set(${LIB_NAME}_FILEPATH_PREFIX "${XMOS_MODULES_ROOT_DIR}/rtos/FreeRTOS/FreeRTOS-Labs/")

list(TRANSFORM LIB_XC_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_C_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_ASM_SRCS PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
list(TRANSFORM LIB_INCLUDES PREPEND ${${LIB_NAME}_FILEPATH_PREFIX})
#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
