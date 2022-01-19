cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## TODO: Specify configuration here
set(MULTITILE_BUILD TRUE)
#set(USE_RTOS_GPIO_DRIVER TRUE)

## Import XMOS RTOS platform configuration.
## Must be done after setting the configuration options.

## TODO: Set the path to your XCORE_SDK_PATH
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

## Include the xmos platform
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

## TODO: Set your project definition here
project(getting_started)

## Enable languages for project
enable_language(CXX C ASM)

## TODO: Modify the compiler flags as needed
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
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/platform/driver_instances.c"
	${XMOS_RTOS_PLATFORM_SOURCES}
    ## TODO: Add your other sources here
)

set(APP_INCLUDES
    "src"
    "src/platform"
	${XMOS_RTOS_PLATFORM_INCLUDES}
)

if(${BOARD} STREQUAL "XCORE200-MIC-ARRAY")
    set(SYSCLK_FREQ 500)
else()
    set(SYSCLK_FREQ 700)
endif()

## TODO: Modify the compile definitions as needed
add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

set(RTOS_TILE_LIST 0 1 )
create_multitile_target(RTOS_TILE_LIST)
