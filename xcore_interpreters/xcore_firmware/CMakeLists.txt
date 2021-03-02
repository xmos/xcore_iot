cmake_minimum_required(VERSION 3.14)

#**********************
# Setup XMOS toolchain
#**********************
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/xmos_toolchain.cmake")

project(xcore_interpreter VERSION 1.0.0)

enable_language(CXX C ASM)

#**********************
# Disable in-source build.
#**********************
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")

#**********************
# Build flags
#**********************
# Optimization 
# -DNDEBUG                        # define this for remove debug and profiling 
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

set(BUILD_FLAGS
  "-fxscope"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
  "-target=XCORE-AI-EXPLORER"
  "-mcmodel=large"
  "-report"
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-fno-rtti"
  "-fno-exceptions"
  "-DXCORE"
  "-DTF_LITE_STATIC_MEMORY"
  "-Os"
)

add_executable(xcore_interpreter)

target_compile_options(xcore_interpreter PRIVATE ${BUILD_FLAGS})

target_link_options(xcore_interpreter PRIVATE ${BUILD_FLAGS} -lquadspi -w -fcmdline-buffer-bytes=2048)

set_target_properties(xcore_interpreter PROPERTIES OUTPUT_NAME xcore_interpreter.xe)

#**********************
# targets
#**********************
include("${PROJECT_SOURCE_DIR}/../../ai_framework.cmake")

target_sources(xcore_interpreter
  PRIVATE ${XCORE_INTERPRETER_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/model_runner.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
)

target_include_directories(xcore_interpreter
  PRIVATE ${XCORE_INTERPRETER_INCLUDES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

install(TARGETS xcore_interpreter DESTINATION ${INSTALL_DIR})
