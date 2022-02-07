cmake_minimum_required(VERSION 3.20)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## in the future will just pass this in
include(tools/cmake_utils/xmos_xs3a_toolchain.cmake)
include(tools/cmake_utils/create_multitile_target.cmake)

## Project declaration
project(xcore_sdk)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(BUILD_TESTS    "Build test applications"    OFF)
option(BUILD_EXAMPLES "Build example applications" OFF)
option(BUILD_HOST     "Build host applications"    OFF)

## TODO REMOVE
set(XCORE_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(modules)

if(BUILD_EXAMPLES)
# add_subdirectory(examples)
message(WARNING "BUILD_EXAMPLES not yet implemented")
endif()
## for testing build the explorer board example
add_subdirectory(examples/freertos/explorer_board)

if(BUILD_TESTS)
# add_subdirectory(test)
message(WARNING "BUILD_TESTS not yet implemented")
endif()

if(BUILD_HOST)
# add_subdirectory(test)
message(WARNING "BUILD_HOST not yet implemented")
endif()
