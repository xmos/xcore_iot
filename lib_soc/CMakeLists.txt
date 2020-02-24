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

set(LIB_NAME lib_soc)
set(LIB_VERSION 0.5.0)
set(LIB_ADD_COMPILER_FLAGS "-Os")
set(LIB_XC_SRCS
        "src/framework/multitile_main.xc"
    )
set(LIB_C_SRCS
        "src/framework/dma_ring_buf.c"
        "src/framework/intertile_channel.c"
        "src/framework/multitile_main.c"
        "src/framework/peripheral_control.c"
        "src/framework/peripheral_hub.c"
        "src/peripherals/bsp/common/freertos/soc_bsp_freertos_common.c"
   )
set(LIB_ASM_SRCS
        ""
    )
set(LIB_INCLUDES
        "src/framework"
        "src/peripherals/bsp/common"
    )
set(LIB_DEPENDENT_MODULES
        "lib_rtos_support(>=0.5.0)"
        "lib_xcore_c(>=2.0.0)"
        "FreeRTOS(>=10.3.0)"
    )
set(LIB_OPTIONAL_HEADERS "soc_conf.h" "rtos_peripherals_conf.h")

#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
set(LIB_SOC_FRAMEWORK ${FULL})

include("src/peripherals/bitstream/bitstream.cmake")
include("src/peripherals/bsp/bsp.cmake")

list(REMOVE_DUPLICATES LIB_ADD_COMPILER_FLAGS)
list(REMOVE_DUPLICATES LIB_XC_SRCS)
list(REMOVE_DUPLICATES LIB_C_SRCS)
list(REMOVE_DUPLICATES LIB_INCLUDES)
list(REMOVE_DUPLICATES LIB_DEPENDENT_MODULES)
list(REMOVE_DUPLICATES LIB_OPTIONAL_HEADERS)

XMOS_REGISTER_MODULE()
