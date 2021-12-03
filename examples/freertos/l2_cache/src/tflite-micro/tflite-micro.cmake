set(TFLITE_MICRO_BENCHMARK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/tflite-micro")
set(TFLITE_MICRO_SOURCE_DIR "${TFLITE_MICRO_BENCHMARK_DIR}/src") 

#**********************
# Paths
#**********************

set(GEMMLOWP_INCLUDE_DIR "${TFLITE_MICRO_SOURCE_DIR}/third_party/gemmlowp")
set(RUY_INCLUDE_DIR "${TFLITE_MICRO_SOURCE_DIR}/third_party/ruy")
set(FLATBUFFERS_INCLUDE_DIR "${TFLITE_MICRO_SOURCE_DIR}/third_party/flatbuffers/include")
set(FLATBUFFERS_SOURCE_DIR "${TFLITE_MICRO_SOURCE_DIR}/third_party/flatbuffers/src")
set(TFLITE_MICRO_INCLUDE_DIR "${TFLITE_MICRO_SOURCE_DIR}")

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

set(TFLITE_MICRO_RUNTIME_SOURCES
    ${TFLITE_MICRO_RUNTIME_SOURCES}
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
    "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
)

#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TFLITE_MICRO_REFERENCE_OPERATOR_SOURCES
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/activations.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/add.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/add_n.cc" 
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/ceil.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/comparisons.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/concatenation.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/conv_common.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/cumsum.cc"  
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depthwise_conv_common.cc"  
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/dequantize.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/detection_postprocess.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/elementwise.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/elu.cc"  
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/l2norm.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/logical.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/logistic.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/maximum_minimum.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/mul.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/neg.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pack.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pad.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pooling.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/prelu.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/quantize.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/reduce.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/reshape.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/shape.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/round.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax_common.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/split.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/split_v.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/squeeze.cc"  
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/sub.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/svdf_common.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/tanh.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/unpack.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor_div.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/floor_mod.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/l2_pool_2d.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/leaky_relu.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/depth_to_space.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/transpose_conv.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/resize_bilinear.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/batch_to_space_nd.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/space_to_batch_nd.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/transpose.cc"
  "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/pooling_common.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/expand_dims.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/space_to_depth.cc"
#   "${TFLITE_MICRO_SOURCE_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc"  
)

set(TFLITE_MICRO_SOURCES
    ${TFLITE_MICRO_RUNTIME_SOURCES}
    ${TFLITE_MICRO_REFERENCE_OPERATOR_SOURCES}
    "${TFLITE_MICRO_BENCHMARK_DIR}/benchmark_tflite-micro.cc"
    "${TFLITE_MICRO_BENCHMARK_DIR}/cifar10_quant.c"
)

## Includes
set(TFLITE_MICRO_INCLUDES 
    ${FLATBUFFERS_INCLUDE_DIR}
    ${GEMMLOWP_INCLUDE_DIR}
    ${RUY_INCLUDE_DIR}
    ${TFLITE_MICRO_INCLUDE_DIR}
    ${TFLITE_MICRO_BENCHMARK_DIR}
)
