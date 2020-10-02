cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_utils.cmake")
set(XMOS_APP_MODULES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

## Define project
project(independent_tiles)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_CXX_SRCS
        ""
)
set(APP_C_SRCS
        "src/main.c"
        "src/intertile_ctrl/intertile_ctrl.c"
        "src/rpc/rpc_test.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
        "src/intertile_ctrl"
        "src/rpc"
    )
set(APP_DEPENDENT_MODULES
        "lib_soc(>=0.5.0)"
    )

set(USE_FREERTOS_SMP False)

## Add lib_soc devices
set(LIB_SOC_INTERTILE ${FULL})
set(LIB_SOC_SW_PIPE_MGR True)
set(LIB_SOC_SW_RPC True)

## Register the application
XMOS_REGISTER_APP()
