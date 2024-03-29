cmake_minimum_required(VERSION 3.21)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Project declaration
project(xcore_iot)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(XCORE_IOT_TESTS     "Enable xcore_iot tests"  OFF)

## Import some helpful macros
include(modules/rtos/tools/cmake_utils/xmos_macros.cmake)

## Setup a framework root path
set(XCORE_IOT_ROOT ${PROJECT_SOURCE_DIR} CACHE STRING "Root folder of xcore_iot in this cmake project tree")

## Add library subdirectories
add_subdirectory(modules)

## Add top level project targets
if(PROJECT_IS_TOP_LEVEL)
    include(examples/examples.cmake)
endif()

if(XCORE_IOT_TESTS)
    include(test/tests.cmake)
endif()
