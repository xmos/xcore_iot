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

project(hello_world VERSION 1.0.0 )

enable_language(CXX C ASM)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "${AIOT_SDK_PATH}/modules/device_memory_support/bare-metal/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "${AIOT_SDK_PATH}/modules/device_memory_support/bare-metal")

#**********************
# APP sources
#**********************
include("${AIOT_SDK_PATH}/modules/aif/ai_framework.cmake")

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
  PRIVATE ${XCORE_INTERPRETER_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(hello_world
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE ${XCORE_INTERPRETER_INCLUDES}
  PRIVATE "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite/micro/examples/hello_world"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS hello_world DESTINATION ${INSTALL_DIR})
