cmake_minimum_required(VERSION 3.14)

set(SDK_PATH $ENV{XCORE_SDK_PATH})

#**********************
# Setup XMOS toolchain
#**********************
include("${SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#**********************
# Project
#**********************
# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

project(micro_speech VERSION 1.0.0 )

enable_language(CXX C ASM)

#**********************
# Paths
#**********************

set(KISS_FFT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/kissfft")

set(RTOS_SUPPORT_DIR "${SDK_PATH}/modules")

set(LIB_DSP_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/lib_dsp/lib_dsp/api")
set(LIB_MIC_ARRAY_INCLUDE_DIR "${RTOS_SUPPORT_DIR}/legacy_compat/lib_mic_array/lib_mic_array/api")

set(LIB_DSP_SOURCE_DIR "${RTOS_SUPPORT_DIR}/lib_dsp/lib_dsp/src")
set(LIB_MIC_ARRAY_SOURCE_DIR "${RTOS_SUPPORT_DIR}/legacy_compat/lib_mic_array")

#**********************
# Build flags
#**********************
add_executable(micro_speech)
add_executable(micro_speech_test)

# Optimization
# -DNDEBUG                        # define this to remove debug and profiling
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

if(DEFINED BOARD)
    set(BOARD_FLAG "${CMAKE_SOURCE_DIR}/board_support/${BOARD}.xn")
else()
    message(FATAL_ERROR "\n-DBOARD must be specified.")
endif()

set(BUILD_FLAGS
${BOARD_FLAG}
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

include("${SDK_PATH}/modules/aif/ai_framework.cmake")

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
# lib_mic_array sources
#**********************
set(LIB_MIC_ARRAY_SOURCES
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/mic_array_dual.xc"
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/fir/fir_coefs_dual.xc"
  "${LIB_MIC_ARRAY_SOURCE_DIR}/lib_mic_array/src/fir/fir_coefs.xc"
)

#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "${SDK_PATH}/modules/device_memory_support/bare-metal/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "${SDK_PATH}/modules/device_memory_support/bare-metal")

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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/system_setup.cc"
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
  ${XCORE_INTERPRETER_SOURCES}
)

target_sources(micro_speech
  PRIVATE ${COMMON_TARGET_SOURCES}
  PRIVATE ${KISS_FFT_SOURCES}
  PRIVATE ${MICRO_FEATURES_SOURCES}
  PRIVATE ${LIB_DSP_SOURCES}
  PRIVATE ${LIB_MIC_ARRAY_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_sources(micro_speech_test
  PRIVATE ${COMMON_TARGET_SOURCES}
  PRIVATE ${TEST_SOURCES}
)

set(COMMON_TARGET_INCLUDES
  ${ADDITIONAL_INCLUDE_DIR}
  ${XCORE_INTERPRETER_INCLUDES}
  ${KISS_FFT_INCLUDE_DIR}
  ${LIB_DSP_INCLUDE_DIR}
  ${LIB_MIC_ARRAY_INCLUDE_DIR}
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
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin/${BOARD}")
install(TARGETS micro_speech micro_speech_test DESTINATION ${INSTALL_DIR})
