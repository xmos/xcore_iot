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

set(LIB_NAME lib_ethernet)
set(LIB_VERSION 3.4.0)
set(LIB_ADD_COMPILER_FLAGS "-g" "-O3" "-mno-dual-issue" "-Wno-unusual-code" "-Wno-cast-align" "-Wno-reinterpret-alignment")
set(LIB_XC_SRCS
        "lib_ethernet/lib_ethernet/src/client_state.xc"
        "lib_ethernet/lib_ethernet/src/ethernet.xc"
        "lib_ethernet/lib_ethernet/src/macaddr_filter.xc"
        "lib_ethernet/lib_ethernet/src/mii.xc"
        "lib_ethernet/lib_ethernet/src/mii_ethernet_mac.xc"
        "lib_ethernet/lib_ethernet/src/mii_ethernet_rt_mac.xc"
        "lib_ethernet/lib_ethernet/src/mii_filter.xc"
        "lib_ethernet/lib_ethernet/src/mii_lite_driver.xc"
        "lib_ethernet/lib_ethernet/src/mii_master.xc"
        "lib_ethernet/lib_ethernet/src/rgmii_10_100_master.xc"
        "lib_ethernet/lib_ethernet/src/rgmii_buffering.xc"
        "lib_ethernet/lib_ethernet/src/rgmii_ethernet_mac.xc"
        "lib_ethernet/lib_ethernet/src/server_state.xc"
        "lib_ethernet/lib_ethernet/src/smi.xc"
    )
set(LIB_CXX_SRCS
        ""
    )
set(LIB_C_SRCS
        "lib_ethernet/lib_ethernet/src/mii_buffering.c"
        "lib_ethernet/lib_ethernet/src/mii_ts_queue.c"
        "lib_ethernet/lib_ethernet/src/rgmii_buffering_c_support.c"
    )
set(LIB_ASM_SRCS
        "lib_ethernet/lib_ethernet/src/mii_common_lld.S"
        "lib_ethernet/lib_ethernet/src/mii_lite_interrupt.S"
        "lib_ethernet/lib_ethernet/src/mii_lite_lld.S"
        "lib_ethernet/lib_ethernet/src/rgmii_rx_lld.S"
        "lib_ethernet/lib_ethernet/src/rgmii_speed_handlers.S"
        "lib_ethernet/lib_ethernet/src/rgmii_tx_lld.S"
    )
set(LIB_INCLUDES
        "lib_ethernet/lib_ethernet/api"
        "lib_ethernet/lib_ethernet/src"
    )
set(LIB_DEPENDENT_MODULES
        "lib_gpio(>=1.1.0)"
        "lib_locks(>=2.0.0)"
        "lib_logging(>=2.1.0)"
        "lib_xassert(>=3.0.0)"
    )
set(LIB_OPTIONAL_HEADERS "ethernet_conf.h")

# Set file specific compiler flags
XMOS_ADD_FILE_COMPILER_FLAGS(lib_ethernet/lib_ethernet/src/mii_master.xc "-O3 -fschedule -g0 -mno-dual-issue")
XMOS_ADD_FILE_COMPILER_FLAGS(lib_ethernet/lib_ethernet/src/macaddr_filter.xc "-Wno-cast-align")
XMOS_ADD_FILE_COMPILER_FLAGS(lib_ethernet/lib_ethernet/src/mii.xc "-Wno-reinterpret-alignment -Wno-unusual-code")
XMOS_ADD_FILE_COMPILER_FLAGS(lib_ethernet/lib_ethernet/src/mii_ethernet_mac.xc "-Wno-unusual-code")
XMOS_ADD_FILE_COMPILER_FLAGS(lib_ethernet/lib_ethernet/src/ethernet.xc "-Wno-cast-align")

#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
