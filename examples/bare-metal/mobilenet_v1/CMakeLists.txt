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
add_library(inference_engine STATIC)

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

target_compile_options(inference_engine PRIVATE ${BUILD_FLAGS})
target_compile_options(mobilenet_v1 PRIVATE ${BUILD_FLAGS})

target_link_options(mobilenet_v1 PRIVATE ${BUILD_FLAGS} -lquadspi -w)
set_target_properties(mobilenet_v1 PROPERTIES OUTPUT_NAME mobilenet_v1.xe)

target_link_libraries(mobilenet_v1 PUBLIC inference_engine)

#**********************
# LIB sources
#**********************

include("${AI_TOOLS_DIR}/utils/cmake/xcore_interpreter.cmake")

set(INFERENCE_ENGINE_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/inference_engine/src"
)

set(INFERENCE_ENGINE_SOURCES
  ${XCORE_INTERPRETER_SOURCES}
  "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash/xcore_device_memory.c"
  "${INFERENCE_ENGINE_SOURCE_DIR}/inference_engine.cc"
  "${INFERENCE_ENGINE_SOURCE_DIR}/mobilenet_v1.c"
)

set(INFERENCE_ENGINE_INCLUDE_DIRS
  ${XCORE_INTERPRETER_INCLUDE_DIRS}
  "$ENV{XMOS_AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash"
  ${INFERENCE_ENGINE_SOURCE_DIR}
)

target_sources(inference_engine PRIVATE ${INFERENCE_ENGINE_SOURCES})
target_include_directories(inference_engine PRIVATE ${INFERENCE_ENGINE_INCLUDE_DIRS})

#**********************
# APP sources
#**********************

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/app/main.xc"
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

set_source_files_properties("${PROJECT_SOURCE_DIR}/app/main.xc" PROPERTIES LANGUAGE C)

#**********************
# targets
#**********************
target_sources(mobilenet_v1
  PRIVATE ${APP_SOURCES}
)

target_include_directories(mobilenet_v1
  PRIVATE ${INFERENCE_ENGINE_INCLUDE_DIRS}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app/src"
)

#**********************
# install
#**********************
set(INSTALL_LIB_DIR "${PROJECT_SOURCE_DIR}/lib")
set(INSTALL_BIN_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS mobilenet_v1 DESTINATION ${INSTALL_BIN_DIR})
install(TARGETS inference_engine DESTINATION ${INSTALL_LIB_DIR})
