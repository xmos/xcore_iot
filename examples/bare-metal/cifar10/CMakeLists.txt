cmake_minimum_required(VERSION 3.14)

set(AIOT_SDK_PATH $ENV{XMOS_AIOT_SDK_PATH})

#**********************
# Setup XMOS toolchain
#**********************

include("${AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

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
  "-fno-rtti"
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
  "${AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "${AIOT_SDK_PATH}/modules/tensorflow_support/qspi_flash")

#**********************
# APP sources
#**********************
include("${AIOT_SDK_PATH}/modules/drivers/sw_services/model_runner/model_runner.cmake")

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/model_runner/cifar10_model_data.c"
  "${PROJECT_SOURCE_DIR}/model_runner/cifar10_model_runner.cc"
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

#**********************
# targets
#**********************
target_sources(cifar10
  PRIVATE ${APP_SOURCES}
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${MODEL_RUNNER_SOURCES}
)

target_include_directories(cifar10
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/model_runner"
  PRIVATE ${MODEL_RUNNER_INCLUDES}
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE "${AIOT_SDK_PATH}/modules"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS cifar10 DESTINATION ${INSTALL_DIR})
