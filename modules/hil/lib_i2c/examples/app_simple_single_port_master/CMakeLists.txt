cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import compiler
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

## Import hil source
set(USE_I2C_HIL TRUE)
set(USE_I2S_HIL FALSE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

## Define project
project(app_simple_single_port_master)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX C ASM)

set(APP_NAME app_simple_single_port_master)

set(APP_COMPILER_FLAGS
    "-O2"
    "-g"
    "-report"
    "-target=XCORE-AI-EXPLORER"
)

set(APP_SOURCES
    "src/main.c"
)

set(APP_INCLUDES
    "src"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(TARGET_NAME "${APP_NAME}.xe")
add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2C_HIL_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2C_HIL_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})

target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR})
