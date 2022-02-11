cmake_minimum_required(VERSION 3.20)

#**********************
# Paths
#**********************

set(GEMMLOWP_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/gemmlowp")
set(RUY_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/ruy")
set(FLATBUFFERS_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/flatbuffers/include")
set(FLATBUFFERS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/flatbuffers/src")
set(TFLITE_MICRO_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tflite-micro")
set(TFLITE_MICRO_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/tflite-micro")

set(LIB_NN_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn")
set(LIB_NN_ALT_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn/lib_nn/api")
set(LIB_NN_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib_nn")

set(MODEL_RUNNER_DIR "${CMAKE_CURRENT_LIST_DIR}/model_runner")

#********************************
# TensorFlow Lite Micro sources
#********************************
set(TFLITE_MICRO_RUNTIME_SOURCES
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/c/common.c"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/core/api/error_reporter.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/core/api/op_resolver.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/core/api/tensor_utils.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/kernels/kernel_util.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/flatbuffer_utils.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/memory_helpers.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_allocator.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_error_reporter.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_graph.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_interpreter.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_profiler.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_utils.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_string.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/recording_micro_allocator.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/recording_simple_memory_allocator.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/simple_memory_allocator.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/quantize_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/schema/schema_utils.cc"
  )

if (X86)
  set(TFLITE_MICRO_RUNTIME_SOURCES
    ${TFLITE_MICRO_RUNTIME_SOURCES}
    "${FLATBUFFERS_SOURCE_DIR}/util.cpp"
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/debug_log.cc"
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/micro_time.cc"
    )
else ()
  set(TFLITE_MICRO_RUNTIME_SOURCES
    ${TFLITE_MICRO_RUNTIME_SOURCES}
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
    )
endif ()

#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TFLITE_MICRO_REFERENCE_OPERATOR_SOURCES
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/activations.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/add.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/add_n.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/ceil.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/comparisons.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/concatenation.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/cumsum.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/dequantize.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/detection_postprocess.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/elementwise.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/elu.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/l2norm.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/logical.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/logistic.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/maximum_minimum.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/mul.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/neg.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pack.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pad.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pooling.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/prelu.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/quantize.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/reduce.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/reshape.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/shape.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/round.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/split.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/split_v.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/squeeze.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/sub.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/tanh.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/unpack.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor_div.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor_mod.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/l2_pool_2d.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/leaky_relu.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depth_to_space.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/transpose_conv.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_bilinear.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/batch_to_space_nd.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/space_to_batch_nd.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/transpose.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pooling_common.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/expand_dims.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/space_to_depth.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
)

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TFLITE_MICRO_XCORE_OPERATOR_SOURCES
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_activations.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_add.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_arg_min_max.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_avgpool_global.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bconv2d.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_bsign.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_conv2d.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_memory_loader.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_op_utils.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pad.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_pooling.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc"
)

#**********************
# lib_nn sources
#**********************
file(GLOB_RECURSE LIB_NN_SOURCES_C "${LIB_NN_SOURCE_DIR}/lib_nn/src/*.c")
file(GLOB_RECURSE LIB_NN_SOURCES_ASM "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/*.S")

#**************************************
# set XCORE_RUNTIME user variables
#**************************************

if (X86)
  set(XCORE_RUNTIME_SOURCES
      ${LIB_NN_SOURCES_C}
      ${TFLITE_MICRO_RUNTIME_SOURCES}
      ${TFLITE_MICRO_REFERENCE_OPERATOR_SOURCES}
      ${TFLITE_MICRO_XCORE_OPERATOR_SOURCES}
  )
else ()
  set(XCORE_RUNTIME_SOURCES
    ${TFLITE_MICRO_RUNTIME_SOURCES}
    ${TFLITE_MICRO_REFERENCE_OPERATOR_SOURCES}
    ${TFLITE_MICRO_XCORE_OPERATOR_SOURCES}
    ${LIB_NN_SOURCES_C}
    ${LIB_NN_SOURCES_ASM}
  )
endif ()

set(XCORE_RUNTIME_INCLUDES
  ${FLATBUFFERS_INCLUDE_DIR}
  ${GEMMLOWP_INCLUDE_DIR}
  ${RUY_INCLUDE_DIR}
  ${TFLITE_MICRO_INCLUDE_DIR}
  ${LIB_NN_ALT_INCLUDE_DIR}
  ${LIB_NN_INCLUDE_DIR}
)

list(REMOVE_DUPLICATES XCORE_RUNTIME_SOURCES)
list(REMOVE_DUPLICATES XCORE_RUNTIME_INCLUDES)

#**************************************
# set MODEL_RUNNER user variables
#**************************************

set(MODEL_RUNNER_SOURCES
    ${XCORE_RUNTIME_SOURCES}
    "${MODEL_RUNNER_DIR}/src/model_runner.cc"
)

if (DEFINED RTOS_CMAKE_RTOS)
  if (${RTOS_CMAKE_RTOS} STREQUAL "FreeRTOS")
    set(MODEL_RUNNER_SOURCES
      ${MODEL_RUNNER_SOURCES}
      "${MODEL_RUNNER_DIR}/src/rtos_dispatcher.cc"
    )
  endif ()
endif ()

set(MODEL_RUNNER_INCLUDES
  ${XCORE_RUNTIME_INCLUDES}
  "${MODEL_RUNNER_DIR}/api"
)

list(REMOVE_DUPLICATES MODEL_RUNNER_SOURCES)
list(REMOVE_DUPLICATES MODEL_RUNNER_INCLUDES)

#***************************
# set source file properties
#***************************

# suppress fptrgroup warnings for now
if (NOT X86)
  set_source_files_properties(${XCORE_RUNTIME_SOURCES} PROPERTIES COMPILE_FLAGS -Wno-xcore-fptrgroup)
  set_source_files_properties(${MODEL_RUNNER_SOURCES} PROPERTIES COMPILE_FLAGS -Wno-xcore-fptrgroup)
endif ()
