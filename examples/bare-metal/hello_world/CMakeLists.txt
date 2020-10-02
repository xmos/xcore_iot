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

project(hello_world VERSION 1.0.0 )

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
add_executable(hello_world)

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
"-Os"
)
target_link_options(hello_world PRIVATE ${BUILD_FLAGS} -lquadspi)
set_target_properties(hello_world PROPERTIES OUTPUT_NAME hello_world.xe)

target_compile_options(hello_world PRIVATE ${BUILD_FLAGS})

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
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world/main.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world/constants.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world/constants.cc"
  "${PROJECT_SOURCE_DIR}/model.c"
  "${PROJECT_SOURCE_DIR}/output_handler.cc"
  "${PROJECT_SOURCE_DIR}/main_functions.cc"
)

set(TEST_APP_SOURCES
  ${APP_SOURCES}
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world/hello_world_test.cc"
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world/output_handler_test.cc"
)

#**********************
# targets
#**********************
target_sources(hello_world
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${LIB_NN_SOURCES}
  PRIVATE ${TENSORFLOW_LITE_SOURCES}
  PRIVATE ${TENSORFLOW_LITE_XCORE_OPERATOR_SOURCES}
  PRIVATE ${INFERENCE_ENGINE_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(hello_world
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE ${GEMMLOWP_INCLUDE_DIR}
  PRIVATE ${RUY_INCLUDE_DIR}
  PRIVATE ${TENSORFLOW_INCLUDE_DIR}
  PRIVATE ${FLATBUFFERS_INCLUDE_DIR}
  PRIVATE ${LIB_NN_ALT_INCLUDE_DIR}
  PRIVATE ${LIB_NN_INCLUDE_DIR}
  PRIVATE "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS hello_world DESTINATION ${INSTALL_DIR})
