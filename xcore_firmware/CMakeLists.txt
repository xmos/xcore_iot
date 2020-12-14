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
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-report"
  "-DXCORE"
  "-Os"
)

add_executable(xcore_interpreter_xscope)
add_executable(xcore_interpreter_cmdline)

target_compile_options(xcore_interpreter_xscope PRIVATE ${BUILD_FLAGS})
target_compile_options(xcore_interpreter_cmdline PRIVATE ${BUILD_FLAGS})

target_link_options(xcore_interpreter_xscope PRIVATE ${BUILD_FLAGS} -lquadspi -w)
target_link_options(xcore_interpreter_cmdline PRIVATE ${BUILD_FLAGS} -lquadspi -w -fcmdline-buffer-bytes=2048)

set_target_properties(xcore_interpreter_xscope PROPERTIES OUTPUT_NAME xcore_interpreter_xscope.xe)
set_target_properties(xcore_interpreter_cmdline PROPERTIES OUTPUT_NAME xcore_interpreter_cmdline.xe)

#**********************
# targets
#**********************
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/xcore_interpreter.cmake")

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/main.xc" PROPERTIES LANGUAGE C)

target_sources(xcore_interpreter_xscope
  PRIVATE ${XCORE_INTERPRETER_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main_xscope.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/model_runner.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.xc"
)

target_sources(xcore_interpreter_cmdline
  PRIVATE ${XCORE_INTERPRETER_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main_cmdline.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/model_runner.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
)

target_include_directories(xcore_interpreter_xscope
  PRIVATE ${XCORE_INTERPRETER_INCLUDE_DIRS}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_include_directories(xcore_interpreter_cmdline
  PRIVATE ${XCORE_INTERPRETER_INCLUDE_DIRS}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

install(TARGETS xcore_interpreter_xscope xcore_interpreter_cmdline DESTINATION ${INSTALL_DIR})
