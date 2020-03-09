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

set(LIB_NAME lib_dsp)
set(LIB_VERSION 6.0.0)
set(LIB_ADD_COMPILER_FLAGS "")
set(LIB_XC_SRCS
        "lib_dsp/lib_dsp/src/dsp_complex.xc"
        "lib_dsp/lib_dsp/src/dsp_dct.xc"
        "lib_dsp/lib_dsp/src/dsp_fast_float.xc"
        "lib_dsp/lib_dsp/src/dsp_idct.xc"
        "lib_dsp/lib_dsp/src/dsp_math_int.xc"
        "lib_dsp/lib_dsp/src/dsp_sqrt.xc"
        "lib_dsp/lib_dsp/src/fft/dsp_fft.xc"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_real.xc"
        "lib_dsp/lib_dsp/src/float/floating_fft.xc"
        "lib_dsp/lib_dsp/src/testing/crc32_c.xc"
    )
set(LIB_C_SRCS
        "lib_dsp/lib_dsp/src/dsp_adaptive.c"
        "lib_dsp/lib_dsp/src/dsp_design.c"
        "lib_dsp/lib_dsp/src/dsp_filters.c"
        "lib_dsp/lib_dsp/src/dsp_math.c"
        "lib_dsp/lib_dsp/src/dsp_matrix.c"
        "lib_dsp/lib_dsp/src/dsp_statistics.c"
        "lib_dsp/lib_dsp/src/dsp_tables.c"
        "lib_dsp/lib_dsp/src/dsp_vector.c"
        "lib_dsp/lib_dsp/src/poly_eval.c"
        "lib_dsp/lib_dsp/src/bfp/dsp_bfp.c"
        "lib_dsp/lib_dsp/src/bfp/dsp_bfp_vector.c"
        "lib_dsp/lib_dsp/src/float/dsp_filters_fp.c"
        "lib_dsp/lib_dsp/src/float/dsp_fp_vector.c"
        "lib_dsp/lib_dsp/src/float/dsp_poly_eval_fp.c"
        "lib_dsp/lib_dsp/src/testing/dsp_testing_conv.c"
        "lib_dsp/lib_dsp/src/testing/dsp_testing_diff.c"
        "lib_dsp/lib_dsp/src/testing/dsp_testing_print.c"
        "lib_dsp/lib_dsp/src/testing/dsp_testing_rand.c"
    )
set(LIB_ASM_SRCS
        "lib_dsp/lib_dsp/src/dsp_biquad.S"
        "lib_dsp/lib_dsp/src/dsp_complex_add_vector.S"
        "lib_dsp/lib_dsp/src/dsp_complex_fir.S"
        "lib_dsp/lib_dsp/src/dsp_complex_hann.S"
        "lib_dsp/lib_dsp/src/dsp_complex_mul_vector.S"
        "lib_dsp/lib_dsp/src/dsp_fast_atan.S"
        "lib_dsp/lib_dsp/src/dsp_logistics.S"
        "lib_dsp/lib_dsp/src/dsp_min_vector.S"
        "lib_dsp/lib_dsp/src/dsp_poly_eval.S"
        "lib_dsp/lib_dsp/src/dsp_sqrt_xs2.S"
        "lib_dsp/lib_dsp/src/bfp/dsp_bfp_bit_reverse_shl.S"
        "lib_dsp/lib_dsp/src/bfp/dsp_bfp_cls.S"
        "lib_dsp/lib_dsp/src/bfp/dsp_bfp_shl.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_bit_reverse.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_forward.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_gc.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_inverse.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_inverse_DIF.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_merge_spectra.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_real_fix.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_split_spectrum.S"
        "lib_dsp/lib_dsp/src/fft/dsp_fft_zero_reverse_forward.S"
        "lib_dsp/lib_dsp/src/fft/dsp_long_short_conversion.S"
    )
set(LIB_INCLUDES
        "lib_dsp/lib_dsp/api"
    )
set(LIB_DEPENDENT_MODULES "lib_logging(>=3.0.0)")
set(LIB_OPTIONAL_HEADERS "debug_conf.h" "xmos_dsp_elements.h")

#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()

