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

project(cifar10 VERSION 1.0.0)

enable_language(CXX C ASM)


#**********************
# Paths
#**********************

set(AI_TOOLS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/tools/ai_tools")

#**********************
# Build flags
#**********************

add_executable(cifar10)

# Optimization
# -DNDEBUG                        # define this to remove debug and profiling
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

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
if (${USE_SWMEM})
  add_definitions(-DUSE_SWMEM)
elseif (${USE_EXTMEM})
  add_definitions(-DUSE_EXTMEM)
endif ()

target_compile_options(cifar10 PRIVATE ${BUILD_FLAGS})

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

set(MODEL_RUNNER_SOURCE_DIR
  "${PROJECT_SOURCE_DIR}/model_runner/src"
)

set(MODEL_RUNNER_SOURCES
  ${XCORE_INTERPRETER_SOURCES}
  "${MODEL_RUNNER_SOURCE_DIR}/cifar10_model_runner.cc"
  "${MODEL_RUNNER_SOURCE_DIR}/cifar10_model_data.c"
)

set(MODEL_RUNNER_INCLUDE_DIRS
  ${XCORE_INTERPRETER_INCLUDE_DIRS}
  ${MODEL_RUNNER_SOURCE_DIR}
)

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

#**********************
# targets
#**********************
target_sources(cifar10
  PRIVATE ${MODEL_RUNNER_SOURCES}
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(cifar10
  PRIVATE ${MODEL_RUNNER_INCLUDE_DIRS}
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS cifar10 DESTINATION ${INSTALL_DIR})
