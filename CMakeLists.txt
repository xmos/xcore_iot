cmake_minimum_required(VERSION 3.21)

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

## Add library usage targets
add_subdirectory(modules)

## Add top level project targets
if(PROJECT_IS_TOP_LEVEL)
    include(applications/applications.cmake)
    include(examples/examples.cmake)

    if(XCORE_SDK_CI_TESTING)
        include(test/tests.cmake)
    endif()
endif()
