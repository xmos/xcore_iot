
SET(LIB_TFLITE_MICRO_PATH lib_tflite_micro/lib_tflite_micro)
SET(LIB_TFLITE_MICRO_SUBMODULES_PATH ${LIB_TFLITE_MICRO_PATH}/submodules)

## ****************************************
## Create tflite-micro microfrontend target
## ****************************************
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Create library target for the tflite-micro microfrontend
    add_library(xcore_sdk_inferencing_microfrontend STATIC)

    ## Source files
    set(MICROFRONTEND_PRIVATE_SOURCES
        "kissfft/kiss_fft.c"
        "kissfft/tools/kiss_fftr.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft.cc"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_lut.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window.c"
        "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_util.c"
    )

    target_sources(xcore_sdk_inferencing_microfrontend
        PRIVATE
            ${MICROFRONTEND_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_inferencing_microfrontend
        PUBLIC
            "${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro"
            kissfft
    )

    target_compile_definitions(xcore_sdk_inferencing_microfrontend
        PUBLIC
            FIXED_POINT=16
    )

    ## Create an alias
    add_library(sdk::inferencing::microfrontend ALIAS xcore_sdk_inferencing_microfrontend)
endif()

## **********************************
## Create tflite-micro runtime target
## **********************************
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    add_library(xcore_sdk_inferencing_tflite_micro STATIC)
    target_sources(xcore_sdk_inferencing_tflite_micro
        PRIVATE
            ## tflite-micro runtime sources
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/c/common.c
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/core/api/error_reporter.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/core/api/flatbuffer_conversions.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/core/api/op_resolver.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/core/api/tensor_utils.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/kernels/kernel_util.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/kernels/internal/quantization_util.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/flatbuffer_utils.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/memory_helpers.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_allocator.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_error_reporter.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_graph.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_interpreter.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_profiler.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_utils.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/micro_string.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/recording_micro_allocator.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/recording_simple_memory_allocator.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/simple_memory_allocator.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/schema/schema_utils.cc
            ## tflite-micro operator sources
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/kernel_util.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/activations.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/activations_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/add.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/add_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/add_n.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/arg_min_max.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/ceil.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/comparisons.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/concatenation.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/conv.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/conv_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/cumsum.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/depth_to_space.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/dequantize.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/dequantize_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/detection_postprocess.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/elementwise.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/elu.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/expand_dims.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/floor.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/floor_div.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/floor_mod.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/fully_connected.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/hard_swish.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/hard_swish_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/l2norm.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/l2_pool_2d.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/logical.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/logical_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/logistic.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/logistic_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/maximum_minimum.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/mul.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/mul_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/neg.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/pack.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/pad.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/pooling.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/pooling_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/prelu.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/prelu_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/quantize.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/reduce.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/reshape.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/resize_bilinear.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/round.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/shape.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/softmax.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/softmax_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/space_to_depth.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/split.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/split_v.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/squeeze.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/strided_slice.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/sub.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/sub_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/svdf.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/svdf_common.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/tanh.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/transpose.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/transpose_conv.cc
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels/unpack.cc
        )
    target_include_directories(xcore_sdk_inferencing_tflite_micro
        PUBLIC
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/gemmlowp
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/ruy
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/flatbuffers/include
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro
            ${LIB_TFLITE_MICRO_SUBMODULES_PATH}/tflite-micro/tensorflow/lite/micro/kernels    
    )
    target_compile_options(xcore_sdk_inferencing_tflite_micro 
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )  
    target_compile_definitions(xcore_sdk_inferencing_tflite_micro 
        PUBLIC
            "TF_LITE_STATIC_MEMORY"
            $<$<CONFIG:RELEASE>:TF_LITE_STRIP_ERROR_STRINGS>
    )
    ## Create an alias
    add_library(sdk::inferencing::tflite_micro ALIAS xcore_sdk_inferencing_tflite_micro)
endif()

## ********************
## Create lib_nn target
## ********************
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    add_library(xcore_sdk_inferencing_lib_nn STATIC)

    ## Source files
    file(GLOB_RECURSE LIB_NN_SOURCES_ASM lib_nn/lib_nn/src/asm/*.S)
    file(GLOB_RECURSE LIB_NN_SOURCES_C   lib_nn/lib_nn/src/c/*.c lib_nn/lib_nn/src/asm/*.c)
    file(GLOB_RECURSE LIB_NN_SOURCES_CPP lib_nn/lib_nn/src/cpp/*.cpp)

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_NN_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${LIB_NN_SOURCES_ASM})

    ## Gather library sources
    set(LIB_NN_PRIVATE_SOURCES   ${LIB_NN_SOURCES_C} ${LIB_NN_SOURCES_CPP})

    ## Append platform specific sources
    list(APPEND LIB_NN_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

    target_sources(xcore_sdk_inferencing_lib_nn
        PRIVATE
            ${LIB_NN_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_inferencing_lib_nn
        PUBLIC
            lib_nn/lib_nn/api
            lib_nn
    )
    target_compile_options(xcore_sdk_inferencing_lib_nn 
        PRIVATE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )

    ## Create an alias
    add_library(sdk::inferencing::lib_nn ALIAS xcore_sdk_inferencing_lib_nn)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## ******************************
    ## Create lib_tflite_micro target
    ## ******************************
    add_library(xcore_sdk_inferencing_lib_tflite_micro INTERFACE)

    ## Source files
    file(GLOB_RECURSE LIB_TFLITE_MICRO_SOURCES_ASM ${LIB_TFLITE_MICRO_PATH}/src/*.S)
    set_source_files_properties(LIB_TFLITE_MICRO_SOURCES_ASM PROPERTIES LANGUAGE ASM)

    target_sources(xcore_sdk_inferencing_lib_tflite_micro
        INTERFACE
            ${LIB_TFLITE_MICRO_SOURCES_ASM}
            ${LIB_TFLITE_MICRO_PATH}/src/inference_engine.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/micro_time.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_bsign.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_custom_options.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_conv2d_v2.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_error_reporter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_interpreter.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_load_from_flash.cc
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels/xcore_profiler.cc
    )

    target_include_directories(xcore_sdk_inferencing_lib_tflite_micro
        INTERFACE
            ${LIB_TFLITE_MICRO_PATH}/api
            ${LIB_TFLITE_MICRO_PATH}/src
            ${LIB_TFLITE_MICRO_PATH}/src/tflite-xcore-kernels
    )
    target_link_libraries(xcore_sdk_inferencing_lib_tflite_micro
        INTERFACE
            sdk::inferencing::lib_nn
            sdk::inferencing::tflite_micro
    )
    target_compile_options(xcore_sdk_inferencing_lib_tflite_micro 
        INTERFACE
            "-Os"
            "-Wno-xcore-fptrgroup"
            $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>
    )

    ## Create an alias
    add_library(sdk::inferencing::lib_tflite_micro ALIAS xcore_sdk_inferencing_lib_tflite_micro)
endif()
