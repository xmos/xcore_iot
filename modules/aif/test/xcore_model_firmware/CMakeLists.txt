cmake_minimum_required(VERSION 3.14)

#**********************
# Setup XMOS toolchain
#**********************
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../../tools/cmake_utils/xmos_toolchain.cmake")

project(xcore_model_firmware VERSION 1.0.0)

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
  "${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn"
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

if (${USE_SWMEM})
  add_definitions(-DUSE_SWMEM)
elseif (${USE_EXTMEM})
  add_definitions(-DUSE_EXTMEM)
endif ()

add_executable(xcore_model_firmware)

target_compile_options(xcore_model_firmware PRIVATE ${BUILD_FLAGS})

target_link_options(xcore_model_firmware PRIVATE ${BUILD_FLAGS} -lquadspi -w)

set_target_properties(xcore_model_firmware PROPERTIES OUTPUT_NAME xcore_model_firmware.xe)

#**********************
# targets
#**********************
include("${PROJECT_SOURCE_DIR}/../../ai_framework.cmake")

#**********************
# Additional support sources
#**********************
set(DEVICE_MEMORY_SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../device_memory_support/bare-metal")

set(ADDITIONAL_C_SOURCES
  "${DEVICE_MEMORY_SUPPORT_DIR}/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "${DEVICE_MEMORY_SUPPORT_DIR}")

target_sources(xcore_model_firmware
  PRIVATE ${XCORE_INTERPRETER_SOURCES}
  PRIVATE ${MODEL_RUNNER_SOURCES}
  PRIVATE ${ADDITIONAL_C_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_model_data.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_model_runner.cc"
)

target_include_directories(xcore_model_firmware
  PRIVATE ${XCORE_INTERPRETER_INCLUDES}
  PRIVATE ${MODEL_RUNNER_INCLUDES}
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

install(TARGETS xcore_model_firmware DESTINATION ${INSTALL_DIR})
