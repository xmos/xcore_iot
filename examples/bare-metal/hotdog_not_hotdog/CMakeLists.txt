cmake_minimum_required(VERSION 3.20)

#**********************
# Get path to XCore SDK
#**********************
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

#**********************
# Setup XMOS toolchain
#**********************
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#**********************
# Project
#**********************

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

project(hotdog_not_hotdog VERSION 1.0.0)

enable_language(CXX C ASM)

#**********************
# Build flags
#**********************

add_executable(hotdog_not_hotdog)

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
target_link_options(hotdog_not_hotdog PRIVATE ${BUILD_FLAGS} -lquadspi -w)
set_target_properties(hotdog_not_hotdog PROPERTIES OUTPUT_NAME hotdog_not_hotdog.xe)

target_compile_options(hotdog_not_hotdog PRIVATE ${BUILD_FLAGS})


#**********************
# Additional support sources
#**********************
set(ADDITIONAL_C_SOURCES
  "${XCORE_SDK_PATH}/modules/device_memory_support/bare-metal/xcore_device_memory.c"
)
set(ADDITIONAL_INCLUDE_DIR "${XCORE_SDK_PATH}/modules/device_memory_support/bare-metal")

#**********************
# APP sources
#**********************
include("${XCORE_SDK_PATH}/modules/aif/ai_framework.cmake")

set(APP_SOURCES
  "${PROJECT_SOURCE_DIR}/model_runner/hotdog_not_hotdog_model_data.c"
  "${PROJECT_SOURCE_DIR}/model_runner/hotdog_not_hotdog_model_runner.cc"
  "${PROJECT_SOURCE_DIR}/app/main.xc"
  "${PROJECT_SOURCE_DIR}/app/main.c"
)

#**********************
# targets
#**********************
target_sources(hotdog_not_hotdog
  PRIVATE ${ADDITIONAL_C_SOURCES}
  PRIVATE ${MODEL_RUNNER_SOURCES}
  PRIVATE ${APP_SOURCES}
)

target_include_directories(hotdog_not_hotdog
  PRIVATE ${ADDITIONAL_INCLUDE_DIR}
  PRIVATE "${MODEL_RUNNER_INCLUDES}"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/app/src"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/model_runner"
)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS hotdog_not_hotdog DESTINATION ${INSTALL_DIR})
