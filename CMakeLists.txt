cmake_minimum_required(VERSION 3.20)

## Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Project declaration
project(xcore_sdk)

## Enable languages for project
enable_language(CXX C ASM)

## Project options
option(XCORE_SDK_CI_TESTING     "Enable CI test applications"  OFF)

## Import some helpful macros
include(tools/cmake_utils/xmos_macros.cmake)

## Add various targets
if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(applications)
    add_subdirectory(examples)
endif()
add_subdirectory(modules)

if(XCORE_SDK_CI_TESTING)
    add_subdirectory(test)
endif()
