cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## This app only supports the XCORE-AI-EXPLORER board
set(BOARD XCORE-AI-EXPLORER)

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

## Define project
project(explorer_board)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
)

set(APP_INCLUDES
    "src"
)

set(APP_COMPILE_DEFINITIONS
    "DEBUG_PRINT_ENABLE=1"
    "MIC_DUAL_ENABLED=1"
    "PLATFORM_USES_TILE_0=1"
    "PLATFORM_USES_TILE_1=1"
)

set(APP_LINK_OPTIONS
    "-lquadspi"
)


#**********************
# targets
#**********************
set(TARGET_NAME ${PROJECT_NAME})

add_executable(${TARGET_NAME})
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME}.xe)

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${XMOS_PLATFORM_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${XMOS_PLATFORM_INCLUDES})

target_compile_definitions(${TARGET_NAME} PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${APP_LINK_OPTIONS})

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")
install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR})
