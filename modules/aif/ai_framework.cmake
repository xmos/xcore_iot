cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************

set(GEMMLOWP_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/gemmlowp")
set(RUY_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/ruy")
set(FLATBUFFERS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/flatbuffers/include")
set(FLATBUFFERS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/flatbuffers/src")
set(TENSORFLOW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tensorflow")
set(TENSORFLOW_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/tensorflow")

set(LIB_NN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn")
set(LIB_NN_ALT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn/lib_nn/api")
set(LIB_NN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn")

set(MODEL_RUNNER_DIR "${CMAKE_CURRENT_LIST_DIR}/model_runner")

#********************************
# TensorFlow Lite Micro sources
#********************************
set(TENSORFLOW_LITE_RUNTIME_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/c/common.c"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected_common.cc"    
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/quantize_common.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/schema/schema_utils.cc"
  )

if (X86)
  set(TENSORFLOW_LITE_RUNTIME_SOURCES
    ${TENSORFLOW_LITE_RUNTIME_SOURCES}
    "${FLATBUFFERS_SOURCE_DIR}/util.cpp"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_time.cc"
    )
else ()
  set(TENSORFLOW_LITE_RUNTIME_SOURCES
    ${TENSORFLOW_LITE_RUNTIME_SOURCES}
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
    )
endif ()

#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/add.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/ceil.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/comparisons.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/concatenation.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv_common.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/dequantize.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/detection_postprocess.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax_common.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/split.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/split_v.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/sub.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf_common.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/tanh.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/unpack.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc"
)

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_activations.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_add.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_arg_min_max.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_avgpool_global.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bconv2d.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bsign.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_conv2d.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_memory_loader.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_op_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pad.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pooling.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc"
)

#**********************
# lib_nn sources
#**********************
file(GLOB_RECURSE LIB_NN_SOURCES_C "${LIB_NN_SOURCE_DIR}/lib_nn/src/*.c")
file(GLOB_RECURSE LIB_NN_SOURCES_ASM "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/*.S")

#**************************************
# set XCORE_INTERPRETER user variables
#**************************************

if (X86)
  set(XCORE_INTERPRETER_SOURCES
      ${LIB_NN_SOURCES_C}
      ${TENSORFLOW_LITE_RUNTIME_SOURCES}
      ${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES}
      ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
  )
else ()
  set(XCORE_INTERPRETER_SOURCES
    ${LIB_NN_SOURCES_C}
    ${TENSORFLOW_LITE_RUNTIME_SOURCES}
    ${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES}
    ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
    ${LIB_NN_SOURCES_ASM}
  )
endif ()

set(XCORE_INTERPRETER_INCLUDES
  ${FLATBUFFERS_INCLUDE_DIR}
  ${GEMMLOWP_INCLUDE_DIR}
  ${RUY_INCLUDE_DIR}
  ${TENSORFLOW_INCLUDE_DIR}
  ${LIB_NN_ALT_INCLUDE_DIR}
  ${LIB_NN_INCLUDE_DIR}
)

list(REMOVE_DUPLICATES XCORE_INTERPRETER_SOURCES)
list(REMOVE_DUPLICATES XCORE_INTERPRETER_INCLUDES)

#**************************************
# set MODEL_RUNNER user variables
#**************************************

set(MODEL_RUNNER_SOURCES
    ${TENSORFLOW_LITE_RUNTIME_SOURCES}
    ${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES}
    ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
    ${LIB_NN_SOURCES_C}
    ${LIB_NN_SOURCES_ASM}
    "${MODEL_RUNNER_DIR}/src/model_runner.cc"
)

set(MODEL_RUNNER_INCLUDES
  ${FLATBUFFERS_INCLUDE_DIR}
  ${GEMMLOWP_INCLUDE_DIR}
  ${RUY_INCLUDE_DIR}
  ${TENSORFLOW_INCLUDE_DIR}
  ${LIB_NN_ALT_INCLUDE_DIR}
  ${LIB_NN_INCLUDE_DIR}
  "${MODEL_RUNNER_DIR}/api"
)

list(REMOVE_DUPLICATES MODEL_RUNNER_SOURCES)
list(REMOVE_DUPLICATES MODEL_RUNNER_INCLUDES)

#***************************
# set source file properties
#***************************

# suppress fptrgroup warnings for now
if (NOT X86)
  set_source_files_properties(${XCORE_INTERPRETER_SOURCES} PROPERTIES COMPILE_FLAGS -Wno-xcore-fptrgroup)
  set_source_files_properties(${MODEL_RUNNER_SOURCES} PROPERTIES COMPILE_FLAGS -Wno-xcore-fptrgroup)
endif ()
