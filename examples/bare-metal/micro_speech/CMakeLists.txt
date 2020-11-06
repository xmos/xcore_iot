cmake_minimum_required(VERSION 3.14)

#**********************
# Setup XMOS toolchain
#**********************
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#**********************
# Project
#**********************
# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

project(micro_speech VERSION 1.0.0 )

enable_language(CXX XC C ASM)

#**********************
# Paths
#**********************

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

set(KISS_FFT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/kissfft")

set(RTOS_SUPPORT_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules")

set(LIB_DSP_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/lib_dsp/lib_dsp/lib_dsp/api")
set(LIB_LOGGING_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/lib_logging/lib_logging/lib_logging/api")
set(LIB_MIC_ARRAY_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/lib_mic_array/lib_mic_array/lib_mic_array/api")
set(LIB_XASSERT_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/lib_xassert/lib_xassert/lib_xassert/api")

set(LIB_DSP_SOURCE_DIR "${RTOS_SUPPORT_DIR}/lib_dsp/lib_dsp")
set(LIB_LOGGING_SOURCE_DIR "${RTOS_SUPPORT_DIR}/lib_logging/lib_logging")
set(LIB_MIC_ARRAY_SOURCE_DIR "${RTOS_SUPPORT_DIR}/lib_mic_array/lib_mic_array")
set(LIB_XASSERT_SOURCE_DIR "${RTOS_SUPPORT_DIR}/lib_xassert/lib_xassert")

#**********************
# Build flags
#**********************
add_executable(micro_speech)
add_executable(micro_speech_test)

# Optimization
# -DNDEBUG                        # define this to remove debug and profiling
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

set(BUILD_FLAGS
"-target=XCORE-AI-EXPLORER"
"-mcmodel=large"
"-Wno-xcore-fptrgroup"
"-Wno-unknown-pragmas"
"-report"
"-DTF_LITE_STATIC_MEMORY"
"-DXCORE"
"-DMIC_DUAL_ENABLED=1"
"-Os"
)
target_link_options(micro_speech PRIVATE ${BUILD_FLAGS} -lquadspi)
set_target_properties(micro_speech PROPERTIES OUTPUT_NAME micro_speech.xe)
target_compile_options(micro_speech PRIVATE ${BUILD_FLAGS})

target_link_options(micro_speech_test PRIVATE ${BUILD_FLAGS} -lquadspi)
set_target_properties(micro_speech_test PROPERTIES OUTPUT_NAME micro_speech_test.xe)
target_compile_options(micro_speech_test PRIVATE ${BUILD_FLAGS})

#********************************
# TensorFlow Lite Micro sources
#********************************
set(TENSORFLOW_LITE_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/c/common.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/op_resolver.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/tensor_utils.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/kernel_util.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/fully_connected.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  )

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_profiler.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_interpreter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_planning.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_dispatcher.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_fully_connected.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_type_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/xcore/xcore_custom_options.cc"
)

#**********************
# lib_nn sources
#**********************
set(LIB_NN_SOURCES
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/vpu_memcpy.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fully_connected_16.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/requantize_16_to_8.S"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/asm_constants.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/vpu_sim.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_op_utils.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_operator.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/fully_connected.c"
)

#**********************
# kiss_fft sources
#**********************
set(KISS_FFT_SOURCES
  "${PROJECT_SOURCE_DIR}/kissfft/kiss_fft.c"
  "${PROJECT_SOURCE_DIR}/kissfft/tools/kiss_fftr.c"
)

#**********************
# micro_features sources
#**********************
set(MICRO_FEATURES_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/fft.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/filterbank.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/frontend.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/log_lut.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/log_scale.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/window.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib/window_util.c"
)

#**********************
# lib_dsp sources
#**********************
set(LIB_DSP_SOURCES
  ""
)

#**********************
# lib_logging sources
#**********************
set(LIB_LOGGING_SOURCES
  "${LIB_LOGGING_SOURCE_DIR}/lib_logging/src/debug_printf.c"
)

#**********************
# lib_mic_array sources
#**********************
set(LIB_MIC_ARRAY_SOURCES
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/mic_array_dual.xc"
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/fir/fir_coefs_dual.xc"
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/fir/fir_coefs.xc"
)

#**********************
# lib_xassert sources
#**********************
set(LIB_XASSERT_SOURCES
  "${LIB_XASSERT_SOURCE_DIR}/lib_xassert/src/xassert.xc"
)

#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash")

#**********************
# APP sources
#**********************
set(APP_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/feature_provider.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/recognize_commands.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_features_generator.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.cc"
  "${PROJECT_SOURCE_DIR}/app/audio_provider.cc"
  "${PROJECT_SOURCE_DIR}/app/command_responder.cc"
  "${PROJECT_SOURCE_DIR}/app/fifo.c"
  "${PROJECT_SOURCE_DIR}/app/main_functions.cc"
  "${PROJECT_SOURCE_DIR}/app/microspeech_xcore_support.c"
  "${PROJECT_SOURCE_DIR}/model/model.cc"
  "${PROJECT_SOURCE_DIR}/main.cc"
  "${PROJECT_SOURCE_DIR}/main.xc"
)

#*************************************************
# Test sources
#*************************************************
set(TEST_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/micro_features/no_micro_features_data.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/micro_features/yes_micro_features_data.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.cc"
  "${PROJECT_SOURCE_DIR}/model/model.cc"
  "${PROJECT_SOURCE_DIR}/test/micro_speech_test.cc"
)

#**********************
# targets
#**********************
set(COMMON_TARGET_SOURCES
  ${ADDITIONAL_C_SOURCES}
  ${LIB_NN_SOURCES}
  ${TENSORFLOW_LITE_SOURCES}
  ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
  ${INFERENCE_ENGINE_SOURCES}
)

target_sources(micro_speech
  PRIVATE ${COMMON_TARGET_SOURCES}
  PRIVATE ${KISS_FFT_SOURCES}
  PRIVATE ${MICRO_FEATURES_SOURCES}
  PRIVATE ${LIB_DSP_SOURCES}
  PRIVATE ${LIB_LOGGING_SOURCES}
  PRIVATE ${LIB_MIC_ARRAY_SOURCES}
  PRIVATE ${LIB_XASSERT_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_sources(micro_speech_test
  PRIVATE ${COMMON_TARGET_SOURCES}
  PRIVATE ${TEST_SOURCES}
)

set(COMMON_TARGET_INCLUDES
  ${ADDITIONAL_INCLUDE_DIR}
  ${GEMMLOWP_INCLUDE_DIR}
  ${RUY_INCLUDE_DIR}
  ${TENSORFLOW_INCLUDE_DIR}
  ${FLATBUFFERS_INCLUDE_DIR}
  ${LIB_NN_ALT_INCLUDE_DIR}
  ${LIB_NN_INCLUDE_DIR}
  ${KISS_FFT_INCLUDE_DIR}
  ${LIB_DSP_INCLUDE_DIR}
  ${LIB_LOGGING_INCLUDE_DIR}
  ${LIB_MIC_ARRAY_INCLUDE_DIR}
  ${LIB_XASSERT_INCLUDE_DIR}
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/fir"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/micro_speech"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/experimental/microfrontend/lib"
  "${PROJECT_SOURCE_DIR}/model"
  "${PROJECT_SOURCE_DIR}/app"
)

target_include_directories(micro_speech
  PRIVATE ${COMMON_TARGET_INCLUDES}
)

target_include_directories(micro_speech_test
  PRIVATE ${COMMON_TARGET_INCLUDES}
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS micro_speech micro_speech_test DESTINATION ${INSTALL_DIR})
