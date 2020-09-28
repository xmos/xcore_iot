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

set(AI_TOOLS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/tools/ai_tools")

set(GEMMLOWP_INCLUDE_DIR "${AI_TOOLS_DIR}/third_party/gemmlowp")
set(RUY_INCLUDE_DIR "${AI_TOOLS_DIR}/third_party/ruy")
set(FLATBUFFERS_INCLUDE_DIR "${AI_TOOLS_DIR}/third_party/flatbuffers/include")
set(TENSORFLOW_INCLUDE_DIR "${AI_TOOLS_DIR}/third_party/tensorflow")

set(LIB_NN_INCLUDE_DIR "${AI_TOOLS_DIR}/lib_nn")
set(LIB_NN_ALT_INCLUDE_DIR "${AI_TOOLS_DIR}/lib_nn/lib_nn/api")

set(FLATBUFFERS_SOURCE_DIR "${AI_TOOLS_DIR}/third_party/flatbuffers/src")
set(TENSORFLOW_SOURCE_DIR "${AI_TOOLS_DIR}/third_party/tensorflow")
set(LIB_NN_SOURCE_DIR "${AI_TOOLS_DIR}/lib_nn")

#********************************
# TensorFlow Lite Micro sources
#********************************
set(TENSORFLOW_LITE_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/op_resolver.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/tensor_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/kernel_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_helpers.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_string.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/simple_memory_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
  )

set(TENSORFLOW_LITE_SOURCES_C
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/c/common.c"
  )
#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/add.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/ceil.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/comparisons.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/concatenation.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/dequantize.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/elementwise.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/ethosu.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/l2norm.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/logical.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/logistic.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/maximum_minimum.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/mul.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/neg.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/pack.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/pad.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/pooling.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/prelu.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/reduce.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/reshape.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/round.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/split.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/sub.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/tanh.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/unpack.cc"
)

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_conv2d.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_extended_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pad.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pooling.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc"
)

#**********************
# lib_nn sources
#**********************
set(LIB_NN_SOURCES_C
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/avgpool2d.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/asm_constants.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/asm_symbols.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_1x1.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_deep.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_depthwise.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_im2col.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_shallowin.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/fully_connected.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/maxpool2d.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_op_utils.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_operator_conv.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_operator.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/pad.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/vpu_sim.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/deep/nn_conv2d_hstrip_deep.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/depthwise/nn_conv2d_hstrip_depthwise.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/shallow/nn_conv2d_hstrip_shallowin.c"
)

set(LIB_NN_SOURCES_ASM
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/conv2d_1x1.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d_2x2.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d_global.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/conv2d_1x1.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fc_deepin_shallowout_8.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fc_deepin_shallowout_16.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fully_connected_8.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fully_connected_16.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/lookup8.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/maxpool2d.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/requantize_16_to_8.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/vpu_memcpy.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_deep.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_deep_padded.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_tail_deep.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_tail_deep_padded.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/depthwise/nn_conv2d_hstrip_depthwise_padded.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/depthwise/nn_conv2d_hstrip_depthwise.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_shallowin.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_shallowin_padded.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_tail_shallowin.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_tail_shallowin_padded.S"
)

#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash")


set(LIB_NAME inference_engine_core)
set(LIB_VERSION 0.0.0)
set(LIB_ADD_COMPILER_FLAGS "-Os" )
set(LIB_XC_SRCS
        ""
    )
set(LIB_CXX_SRCS
        "${TENSORFLOW_LITE_SOURCES_CXX}"
        "${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES_CXX}"
        "${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES_CXX}"
    )
set(LIB_C_SRCS
        "${ADDITIONAL_C_SOURCES}"
        "${TENSORFLOW_LITE_SOURCES_C}"
        "${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES_C}"
        "${LIB_NN_SOURCES_C}"
    )
set(LIB_ASM_SRCS
        "${LIB_NN_SOURCES_ASM}"
    )
set(LIB_INCLUDES
        "${ADDITIONAL_INCLUDE_DIR}"
        "${GEMMLOWP_INCLUDE_DIR}"
        "${RUY_INCLUDE_DIR}"
        "${TENSORFLOW_INCLUDE_DIR}"
        "${FLATBUFFERS_INCLUDE_DIR}"
        "${LIB_NN_ALT_INCLUDE_DIR}"
        "${LIB_NN_INCLUDE_DIR}"
    )
set(LIB_DEPENDENT_MODULES "")
set(LIB_OPTIONAL_HEADERS "")

#=============================================================================
# The following part of the CMakeLists includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_REGISTER_MODULE()
