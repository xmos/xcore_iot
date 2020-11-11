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

project(mobilenet_v1 VERSION 1.0.0)

enable_language(CXX XC C ASM)

#**********************
# Paths
#**********************

set(AI_TOOLS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/tools/ai_tools")

#**********************
# Build flags
#**********************

add_executable(mobilenet_v1)

# Optimization
# -DNDEBUG                        # define this to remove debug and profiling
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

set(BUILD_FLAGS
  "-target=XCORE-AI-EXPLORER"
  "-fxscope"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
  "-mcmodel=large"
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-report"
  "-DTF_LITE_STATIC_MEMORY"
  "-DXCORE"
  "-DUSE_EXTMEM"
  "-Os"
)
target_link_options(mobilenet_v1 PRIVATE ${BUILD_FLAGS} -lquadspi -w)
set_target_properties(mobilenet_v1 PROPERTIES OUTPUT_NAME mobilenet_v1.xe)

target_compile_options(mobilenet_v1 PRIVATE ${BUILD_FLAGS})


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
include("${AI_TOOLS_DIR}/utils/cmake/xcore_interpreter.cmake")

set(INFERENCE_ENGINE_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/inference_engine/src"
)

set(INFERENCE_ENGINE_SOURCES
  ${XCORE_INTERPRETER_SOURCES}
  "${INFERENCE_ENGINE_SOURCE_DIR}/inference_engine.cc"
  "${INFERENCE_ENGINE_SOURCE_DIR}/mobilenet_v1.c"
)

set(INFERENCE_ENGINE_INCLUDE_DIRS
  ${XCORE_INTERPRETER_INCLUDE_DIRS}
  ${INFERENCE_ENGINE_SOURCE_DIR}
)

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/app/main.xc"
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

#**********************
# targets
#**********************
target_sources(mobilenet_v1
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${INFERENCE_ENGINE_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(mobilenet_v1
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE "${INFERENCE_ENGINE_INCLUDE_DIRS}"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app/src"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS mobilenet_v1 DESTINATION ${INSTALL_DIR})
