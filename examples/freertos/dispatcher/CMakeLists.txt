cmake_minimum_required(VERSION 3.20)

#**********************
# Disable in-source build.
#**********************
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

#**********************
# Setup project
#**********************

# Specify configuration
set(USE_DISPATCHER TRUE)

#**********************
# Get path to XCore SDK
#**********************
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

# Import XMOS RTOS platform configuration.
# Must be done after setting the configuration options.
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

# Define project
project(dispatcher)

# Enable languages for project
enable_language(CXX C ASM)

#**********************
# Build flags
#**********************
set(APP_COMPILER_FLAGS
  "-mcmodel=large"
  "-fxscope"
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-report"
  "-march=xs3a"
  "-Os"
  "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
)

set(APP_SOURCES
  "src/main.c"
  ${XMOS_RTOS_PLATFORM_SOURCES}
)

set(APP_INCLUDES
  "src"
  ${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(TARGET_NAME dispatcher)
add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_RTOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_RTOS_PLATFORM_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})