cmake_minimum_required(VERSION 3.20)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

if(NOT DEFINED ENV{XCORE_SDK_PATH})
    message(FATAL_ERROR "Environment variable XCORE_SDK_PATH must be set")
endif()

## Fetch dependencies
include(FetchContent)

FetchContent_Declare(
  tools_xmostest
  GIT_REPOSITORY git@github.com:xmos/tools_xmostest.git
  GIT_TAG        1e2d0097624e0df9c92981d2fc652fda3f81b9e2
  SOURCE_DIR     tools_xmostest
)
FetchContent_Populate(tools_xmostest)

## Import XMOS toolchain
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

## Define project
project(hil_tests)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX C ASM)

set(HIL_TEST_LIBS
    "lib_i2c"
    "lib_i2s"
    "lib_spi"
)

foreach(lib ${HIL_TEST_LIBS})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${lib})
endforeach()
