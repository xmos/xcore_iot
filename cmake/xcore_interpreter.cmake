cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************

set(AI_TOOLS_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

set(GEMMLOWP_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/gemmlowp")
set(RUY_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/ruy")
set(FLATBUFFERS_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/flatbuffers/include")
set(FLATBUFFERS_SOURCE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/flatbuffers/src")
set(TENSORFLOW_SOURCE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/tensorflow")
set(TENSORFLOW_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/third_party/tensorflow")

set(LIB_NN_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/lib_nn")
set(LIB_NN_ALT_INCLUDE_DIR "${AI_TOOLS_ROOT_DIR}/lib_nn/lib_nn/api")
set(LIB_NN_SOURCE_DIR "${AI_TOOLS_ROOT_DIR}/lib_nn")

#********************************
# TensorFlow Lite Micro sources
#********************************
set(TENSORFLOW_LITE_RUNTIME_SOURCES_C
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/c/common.c"
  )

set(TENSORFLOW_LITE_RUNTIME_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/op_resolver.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/tensor_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/kernel_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_helpers.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/recording_micro_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_string.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/simple_memory_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/recording_simple_memory_allocator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/schema/schema_utils.cc"
  )

if (X86)
  set(TENSORFLOW_LITE_RUNTIME_SOURCES_CXX
    ${TENSORFLOW_LITE_RUNTIME_SOURCES_CXX}
    "${FLATBUFFERS_SOURCE_DIR}/util.cpp"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_time.cc"
    )
else (X86)
  set(TENSORFLOW_LITE_RUNTIME_SOURCES_CXX
    ${TENSORFLOW_LITE_RUNTIME_SOURCES_CXX}
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
    )
endif (X86)

#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/add.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/ceil.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/comparisons.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/concatenation.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/dequantize.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/elementwise.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/quantize.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/reduce.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/reshape.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/shape.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/round.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/split.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/split_v.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/sub.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/tanh.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/unpack.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc"
)

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES_CXX
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_conv2d.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pooling.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bsign.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bconv2d.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_add.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pad.cc"
)

#**********************
# lib_nn sources
#**********************
file(GLOB_RECURSE LIB_NN_SOURCES_C "${LIB_NN_SOURCE_DIR}/lib_nn/src/*.c")
file(GLOB_RECURSE LIB_NN_SOURCES_ASM "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/*.S")


#**********************
# set user variables
#**********************

set(XCORE_INTERPRETER_SOURCES_C
    ${TENSORFLOW_LITE_RUNTIME_SOURCES_C}
    ${LIB_NN_SOURCES_C}
)
set(XCORE_INTERPRETER_SOURCES_CXX
    ${TENSORFLOW_LITE_RUNTIME_SOURCES_CXX}
    ${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES_CXX}
    ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES_CXX}
)

set(XCORE_INTERPRETER_SOURCES_ASM
    ${LIB_NN_SOURCES_ASM}
)

if (X86)
  set(XCORE_INTERPRETER_SOURCES
      ${XCORE_INTERPRETER_SOURCES_C}
      ${XCORE_INTERPRETER_SOURCES_CXX}
  )
else (X86)
  set(XCORE_INTERPRETER_SOURCES
      ${XCORE_INTERPRETER_SOURCES_C}
      ${XCORE_INTERPRETER_SOURCES_CXX}
      ${XCORE_INTERPRETER_SOURCES_ASM}
  )
endif (X86)

set(XCORE_INTERPRETER_INCLUDE_DIRS
  ${FLATBUFFERS_INCLUDE_DIR}
  ${GEMMLOWP_INCLUDE_DIR}
  ${RUY_INCLUDE_DIR}
  ${TENSORFLOW_INCLUDE_DIR}
  ${LIB_NN_ALT_INCLUDE_DIR}
  ${LIB_NN_INCLUDE_DIR}
)


