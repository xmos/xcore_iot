cmake_minimum_required(VERSION 3.10)

#**********************
# Options
#**********************
OPTION (XCORE "If disabled, will build the x86 version." ON)

#**********************
# Setup XMOS toolchain
#**********************

if (XCORE)
    include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")
endif ()

#**********************
# Project
#**********************

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

project(cifar10 VERSION 1.0.0)

enable_language(CXX C ASM)

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

#**********************
# Build flags
#**********************

add_executable(cifar10)

# Optimization
# -DNDEBUG                        # define this to remove debug and profiling
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

if (XCORE)
  set(BUILD_FLAGS
    "-target=XCORE-AI-EXPLORER"
    "-fxscope"
    "-fcmdline-buffer-bytes=1024"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "-Wno-unknown-pragmas"
    "-report"
    "-DTF_LITE_STATIC_MEMORY"
    "-DXCORE"
    "-Os"
  )
  target_link_options(cifar10 PRIVATE ${BUILD_FLAGS} -lquadspi -w)
  set_target_properties(cifar10 PROPERTIES OUTPUT_NAME cifar10.xe)
else ()
  set(BUILD_FLAGS
    "-g"
    "-DTF_LITE_DISABLE_X86_NEON"
    "-DTF_LITE_STATIC_MEMORY"
    "-O3"
  )
  target_link_libraries(cifar10 stdc++ m pthread)
endif ()

target_compile_options(cifar10 PRIVATE ${BUILD_FLAGS})

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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/c/common.c"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/error_reporter.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/op_resolver.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/core/api/tensor_utils.cc"
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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc"
  )

if (XCORE)
  set(TENSORFLOW_LITE_SOURCES
    ${TENSORFLOW_LITE_SOURCES}
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/xcore/micro_time.cc"
    )
else ()
  set(TENSORFLOW_LITE_SOURCES
    ${TENSORFLOW_LITE_SOURCES}
    "${FLATBUFFERS_SOURCE_DIR}/util.cpp"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/debug_log.cc"
    "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/micro_time.cc"
    )
endif ()

#*************************************************
# TensorFlow Lite Micro reference kernel sources
#*************************************************
set(TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/pad.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/kernels/softmax.cc"
)

#*************************************************
# TensorFlow Lite Micro xcore kernel sources
#*************************************************
set(TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES
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
)

#**********************
# lib_nn sources
#**********************
set(LIB_NN_SOURCES
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/asm_constants.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/vpu_sim.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_op_utils.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_operator.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/nn_operator_conv.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_deep.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_shallowin.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_1x1.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/conv2d_depthwise.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/avgpool2d.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/maxpool2d.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/fully_connected.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/deep/nn_conv2d_hstrip_deep.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/shallow/nn_conv2d_hstrip_shallowin.c"
  "${LIB_NN_SOURCE_DIR}/lib_nn/src/c/util/depthwise/nn_conv2d_hstrip_depthwise.c"
)

if (XCORE)
  set(LIB_NN_SOURCES
    ${LIB_NN_SOURCES}
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/conv2d_1x1.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/fully_connected_16.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/maxpool2d.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d_2x2.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/avgpool2d_global.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/vpu_memcpy.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/requantize_16_to_8.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/lookup8.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_shallowin.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_shallowin_padded.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_tail_shallowin.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/shallow/nn_conv2d_hstrip_tail_shallowin_padded.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_deep.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_deep_padded.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_tail_deep.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/deep/nn_conv2d_hstrip_tail_deep_padded.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/depthwise/nn_conv2d_hstrip_depthwise_padded.S"
    "${LIB_NN_SOURCE_DIR}/lib_nn/src/asm/util/depthwise/nn_conv2d_hstrip_depthwise.S"
    )
endif ()

#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/tensorflow/tensorflow/lite/micro/kernels/xcore/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/tensorflow")

#**********************
# APP sources
#**********************
set(INFERENCE_ENGINE_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/inference_engine/src"
)

set(INFERENCE_ENGINE_SOURCES
  "${INFERENCE_ENGINE_SOURCE_DIR}/inference_engine.cc"
  "${INFERENCE_ENGINE_SOURCE_DIR}/cifar10_model.c"
)

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/app/src/main.c"
)

#if (XCORE)
#  set(APP_SOURCES
#    ${APP_SOURCES}
#    "${PROJECT_SOURCE_DIR}/app/src/main.xc"
#    )
#  set_source_files_properties("${PROJECT_SOURCE_DIR}/app/src/main.xc" PROPERTIES LANGUAGE C)
#endif ()

#**********************
# targets
#**********************
target_sources(cifar10
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${TENSORFLOW_LITE_SOURCES}
  PRIVATE ${TENSORFLOW_LITE_REFERENCE_OPERATOR_SOURCES}
  PRIVATE ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
  PRIVATE ${LIB_NN_SOURCES}
  PRIVATE ${INFERENCE_ENGINE_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(cifar10
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE ${GEMMLOWP_INCLUDE_DIR}
  #PRIVATE ${RUY_INCLUDE_DIR}
  PRIVATE ${TENSORFLOW_INCLUDE_DIR}
  PRIVATE ${FLATBUFFERS_INCLUDE_DIR}
  PRIVATE ${LIB_NN_ALT_INCLUDE_DIR}
  PRIVATE ${LIB_NN_INCLUDE_DIR}
  PRIVATE "${INFERENCE_ENGINE_SOURCE_DIR}"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS cifar10 DESTINATION ${INSTALL_DIR})
