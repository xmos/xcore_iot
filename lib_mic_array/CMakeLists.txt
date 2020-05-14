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

set(LIB_NAME lib_mic_array)
set(LIB_VERSION 4.0.0)
set(LIB_ADD_COMPILER_FLAGS "")
set(LIB_XC_SRCS
        "lib_mic_array/lib_mic_array/src/decimator_interface.xc"
        "lib_mic_array/lib_mic_array/src/hires_delay.xc"
        "lib_mic_array/lib_mic_array/src/pdm.xc"
        "lib_mic_array/lib_mic_array/src/mic_array_dual.xc"
        "lib_mic_array/lib_mic_array/src/fir/fir_coefs.xc"
        "lib_mic_array/lib_mic_array/src/fir/fir_coefs_dual.xc"
    )
set(LIB_C_SRCS
        ""
    )
set(LIB_ASM_SRCS
        "lib_mic_array/lib_mic_array/src/decimate_to_pcm_4ch.S"
        "lib_mic_array/lib_mic_array/src/pdm_rx.S"
    )
set(LIB_INCLUDES
        "lib_mic_array/lib_mic_array/api"
        "lib_mic_array/lib_mic_array/src/fir"
    )
set(LIB_DEPENDENT_MODULES "lib_xassert(>=4.0.0)" "lib_logging(>=3.0.0)" "lib_dsp(>=5.0.0)")
set(LIB_OPTIONAL_HEADERS "")

#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
