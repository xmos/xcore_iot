cmake_minimum_required(VERSION 3.20)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## in the future will just pass this in
include(tools/cmake_utils/xmos_xs3a_toolchain.cmake)

## Project declaration
project(xcore_sdk)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(BUILD_HOST     "Build host applications"    OFF)

## Import some helpful macros
include(tools/cmake_utils/xmos_macros.cmake)

## Add various targets
add_subdirectory(modules)
add_subdirectory(examples)
add_subdirectory(test)

if(BUILD_HOST)
# add_subdirectory(test)
message(WARNING "BUILD_HOST not yet implemented")
endif()
