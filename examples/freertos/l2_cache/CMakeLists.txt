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
set(USE_L2_CACHE_HIL TRUE)
set(USE_RTOS_L2_CACHE_DRIVER TRUE)

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
project(l2_cache)

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
  "-O2"
  "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
)

set(APP_SOURCES
  "src/main.c"
  "src/example_code.c"
  "src/print_info.c"
  ${XMOS_RTOS_PLATFORM_SOURCES}
)

set(APP_INCLUDES
  "src"
  ${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    USE_SWMEM=1
    # L2_CACHE_DEBUG_ON=1
)

# Prevent optimizing these. We want them to take up space.
set_source_files_properties(src/example_code.c PROPERTIES COMPILE_FLAGS -O0)

set(TARGET_NAME l2_cache)
add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
    PRIVATE ${APP_SOURCES}
    PRIVATE ${XMOS_RTOS_PLATFORM_SOURCES}
)

target_include_directories(${TARGET_NAME}
    PRIVATE ${APP_INCLUDES}
    PRIVATE ${XMOS_RTOS_PLATFORM_INCLUDES}
)

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME}.xe)


add_custom_target( flash
  COMMAND xobjdump --strip ${TARGET_NAME}.xe
  COMMAND xobjdump --split ${TARGET_NAME}.xb
  COMMAND xflash --write-all image_n0c0.swmem --target XCORE-AI-EXPLORER
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/
)
add_dependencies(flash ${TARGET_NAME})


add_custom_target( run
  COMMAND xrun --xscope ${TARGET_NAME}.xe
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/
)
add_dependencies(run ${TARGET_NAME})
