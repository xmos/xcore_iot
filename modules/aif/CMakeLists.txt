
## Create library target for the tflite-micro microfrontend
add_library(xcore_sdk_aif_microfrontend STATIC)

## Source files
set(MICROFRONTEND_PRIVATE_SOURCES
  "kissfft/kiss_fft.c"
  "kissfft/tools/kiss_fftr.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft.cc"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_lut.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window.c"
  "tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_util.c"
)

target_sources(xcore_sdk_aif_microfrontend
    PRIVATE
        ${MICROFRONTEND_PRIVATE_SOURCES}
)
target_include_directories(xcore_sdk_aif_microfrontend
    PUBLIC
        kissfft
        tflite-micro
)

## Create an alias
add_library(sdk::aif::microfrontend ALIAS xcore_sdk_aif_microfrontend)

## Create library target
add_library(xcore_sdk_aif_tflite_micro_runtime INTERFACE)
target_sources(xcore_sdk_aif_tflite_micro_runtime
    INTERFACE
        tflite-micro/tensorflow/lite/micro/xcore/debug_log.cc
        tflite-micro/tensorflow/lite/micro/xcore/micro_time.cc
        tflite-micro/tensorflow/lite/c/common.c
        tflite-micro/tensorflow/lite/core/api/error_reporter.cc
        tflite-micro/tensorflow/lite/core/api/flatbuffer_conversions.cc
        tflite-micro/tensorflow/lite/core/api/op_resolver.cc
        tflite-micro/tensorflow/lite/core/api/tensor_utils.cc
        tflite-micro/tensorflow/lite/kernels/kernel_util.cc
        tflite-micro/tensorflow/lite/kernels/internal/quantization_util.cc
        tflite-micro/tensorflow/lite/micro/flatbuffer_utils.cc
        tflite-micro/tensorflow/lite/micro/memory_helpers.cc
        tflite-micro/tensorflow/lite/micro/micro_allocator.cc
        tflite-micro/tensorflow/lite/micro/micro_error_reporter.cc
        tflite-micro/tensorflow/lite/micro/micro_graph.cc
        tflite-micro/tensorflow/lite/micro/micro_interpreter.cc
        tflite-micro/tensorflow/lite/micro/micro_profiler.cc
        tflite-micro/tensorflow/lite/micro/micro_utils.cc
        tflite-micro/tensorflow/lite/micro/micro_string.cc
        tflite-micro/tensorflow/lite/micro/recording_micro_allocator.cc
        tflite-micro/tensorflow/lite/micro/recording_simple_memory_allocator.cc
        tflite-micro/tensorflow/lite/micro/simple_memory_allocator.cc
        tflite-micro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
        tflite-micro/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
        tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
        tflite-micro/tensorflow/lite/micro/kernels/kernel_util.cc
        tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
        tflite-micro/tensorflow/lite/schema/schema_utils.cc
)
target_include_directories(xcore_sdk_aif_tflite_micro_runtime
    INTERFACE
        tflite-micro
)

## Create an alias
add_library(sdk::aif::tflite_micro_runtime ALIAS xcore_sdk_aif_tflite_micro_runtime)


## Create library target
add_library(xcore_sdk_aif_lib_nn STATIC)

## Source files
file(GLOB_RECURSE LIB_NN_SOURCES_C   lib_nn/lib_nn/src/*.c    )
file(GLOB_RECURSE LIB_NN_SOURCES_ASM lib_nn/lib_nn/src/asm/*.S)

## cmake doesn't recognize .S files as assembly by default
set_source_files_properties(LIB_NN_SOURCES_ASM PROPERTIES LANGUAGE ASM)

## Assume all asm is XS3A for now
set(XCORE_XS3A_SOURCES ${LIB_NN_SOURCES_ASM})

## Gather library sources
set(LIB_PRIVATE_SOURCES   ${LIB_NN_SOURCES_C})

## Append platform specific sources
list(APPEND LIB_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

target_sources(xcore_sdk_aif_lib_nn
    PRIVATE
        ${LIB_PRIVATE_SOURCES}
)
target_include_directories(xcore_sdk_aif_lib_nn
    PUBLIC
        lib_nn/lib_nn/api
        lib_nn/lib_nn
)

## Create an alias
add_library(sdk::aif::lib_nn ALIAS xcore_sdk_aif_lib_nn)


if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target
    add_library(xcore_sdk_aif_tflite_micro_xcore_kernels INTERFACE)
    target_sources(xcore_sdk_aif_tflite_micro_xcore_kernels
        INTERFACE
            tflite-micro/tensorflow/lite/micro/kernels/activations.cc
            tflite-micro/tensorflow/lite/micro/kernels/add.cc
            tflite-micro/tensorflow/lite/micro/kernels/add_n.cc
            tflite-micro/tensorflow/lite/micro/kernels/arg_min_max.cc
            tflite-micro/tensorflow/lite/micro/kernels/ceil.cc
            tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
            tflite-micro/tensorflow/lite/micro/kernels/comparisons.cc
            tflite-micro/tensorflow/lite/micro/kernels/concatenation.cc
            tflite-micro/tensorflow/lite/micro/kernels/conv.cc
            tflite-micro/tensorflow/lite/micro/kernels/conv_common.cc
            tflite-micro/tensorflow/lite/micro/kernels/cumsum.cc
            tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv.cc
            tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
            tflite-micro/tensorflow/lite/micro/kernels/dequantize.cc
            tflite-micro/tensorflow/lite/micro/kernels/detection_postprocess.cc
            tflite-micro/tensorflow/lite/micro/kernels/elementwise.cc
            tflite-micro/tensorflow/lite/micro/kernels/elu.cc
            tflite-micro/tensorflow/lite/micro/kernels/floor.cc
            tflite-micro/tensorflow/lite/micro/kernels/fully_connected.cc
            tflite-micro/tensorflow/lite/micro/kernels/l2norm.cc
            tflite-micro/tensorflow/lite/micro/kernels/logical.cc
            tflite-micro/tensorflow/lite/micro/kernels/logistic.cc
            tflite-micro/tensorflow/lite/micro/kernels/maximum_minimum.cc
            tflite-micro/tensorflow/lite/micro/kernels/mul.cc
            tflite-micro/tensorflow/lite/micro/kernels/neg.cc
            tflite-micro/tensorflow/lite/micro/kernels/pack.cc
            tflite-micro/tensorflow/lite/micro/kernels/pad.cc
            tflite-micro/tensorflow/lite/micro/kernels/pooling.cc
            tflite-micro/tensorflow/lite/micro/kernels/prelu.cc
            tflite-micro/tensorflow/lite/micro/kernels/quantize.cc
            tflite-micro/tensorflow/lite/micro/kernels/reduce.cc
            tflite-micro/tensorflow/lite/micro/kernels/reshape.cc
            tflite-micro/tensorflow/lite/micro/kernels/shape.cc
            tflite-micro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
            tflite-micro/tensorflow/lite/micro/kernels/round.cc
            tflite-micro/tensorflow/lite/micro/kernels/softmax.cc
            tflite-micro/tensorflow/lite/micro/kernels/softmax_common.cc
            tflite-micro/tensorflow/lite/micro/kernels/split.cc
            tflite-micro/tensorflow/lite/micro/kernels/split_v.cc
            tflite-micro/tensorflow/lite/micro/kernels/squeeze.cc
            tflite-micro/tensorflow/lite/micro/kernels/strided_slice.cc
            tflite-micro/tensorflow/lite/micro/kernels/sub.cc
            tflite-micro/tensorflow/lite/micro/kernels/svdf.cc
            tflite-micro/tensorflow/lite/micro/kernels/svdf_common.cc
            tflite-micro/tensorflow/lite/micro/kernels/tanh.cc
            tflite-micro/tensorflow/lite/micro/kernels/unpack.cc
            tflite-micro/tensorflow/lite/micro/kernels/hard_swish.cc
            tflite-micro/tensorflow/lite/micro/kernels/floor_div.cc
            tflite-micro/tensorflow/lite/micro/kernels/floor_mod.cc
            tflite-micro/tensorflow/lite/micro/kernels/l2_pool_2d.cc
            tflite-micro/tensorflow/lite/micro/kernels/leaky_relu.cc
            tflite-micro/tensorflow/lite/micro/kernels/depth_to_space.cc
            tflite-micro/tensorflow/lite/micro/kernels/transpose_conv.cc
            tflite-micro/tensorflow/lite/micro/kernels/resize_bilinear.cc
            tflite-micro/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
            tflite-micro/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
            tflite-micro/tensorflow/lite/micro/kernels/transpose.cc
            tflite-micro/tensorflow/lite/micro/kernels/pooling_common.cc
            tflite-micro/tensorflow/lite/micro/kernels/expand_dims.cc
            tflite-micro/tensorflow/lite/micro/kernels/space_to_depth.cc
            tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_activations.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_add.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_arg_min_max.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_avgpool_global.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_bconv2d.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_bsign.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_conv2d.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_memory_loader.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_op_utils.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_pad.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_pooling.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc
            tflite-micro/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc
    )
    target_include_directories(xcore_sdk_aif_tflite_micro_xcore_kernels
        INTERFACE
            tflite-micro
    )
    target_link_libraries(xcore_sdk_aif_tflite_micro_xcore_kernels
        INTERFACE
            sdk::aif::tflite_micro_runtime
            sdk::aif::lib_nn
    )

    ## Create an alias
    add_library(sdk::aif_tflite_micro::xcore_kernels ALIAS xcore_sdk_aif_tflite_micro_xcore_kernels)


    ## Create library target
    add_library(xcore_sdk_aif_model_runner_bare_metal INTERFACE)
    target_sources(xcore_sdk_aif_model_runner_bare_metal
        INTERFACE
            model_runner/src/model_runner.cc
    )
    target_include_directories(xcore_sdk_aif_model_runner_bare_metal
        INTERFACE
            model_runner/api
            model_runner/src
            gemmlowp
            ruy
            flatbuffers/include
    )
    target_link_libraries(xcore_sdk_aif_model_runner_bare_metal
        INTERFACE
            sdk::aif::tflite_micro_runtime
            sdk::aif::lib_nn
            sdk::device_memory_support_bare_metal
            sdk::aif_tflite_micro::xcore_kernels
    )

    ## Create an alias
    add_library(sdk::aif_model_runner_bare_metal ALIAS xcore_sdk_aif_model_runner_bare_metal)


    ## Create library target
    add_library(xcore_sdk_aif_model_runner_freertos INTERFACE)
    target_sources(xcore_sdk_aif_model_runner_freertos
        INTERFACE
            model_runner/src/model_runner.cc
            model_runner/src/rtos_dispatcher.cc
    )
    target_include_directories(xcore_sdk_aif_model_runner_freertos
        INTERFACE
            model_runner/api
            model_runner/src
            gemmlowp
            ruy
            flatbuffers/include
    )
    target_link_libraries(xcore_sdk_aif_model_runner_freertos
        INTERFACE
            sdk::aif::tflite_micro_runtime
            sdk::aif::lib_nn
            sdk::device_memory_support_rtos
            sdk::aif_tflite_micro::xcore_kernels
            sdk::rtos::sw_services::dispatcher
    )
    target_compile_definitions(xcore_sdk_aif_model_runner_freertos
        INTERFACE
            RTOS_FREERTOS=1
    )

    ## Create an alias
    add_library(sdk::aif_model_runner_freertos ALIAS xcore_sdk_aif_model_runner_freertos)
endif()
