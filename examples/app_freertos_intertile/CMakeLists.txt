cmake_minimum_required(VERSION 3.13)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
set(XMOS_MODULES_ROOT_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../..")
include("${XMOS_MODULES_ROOT_DIR}/cmake_utils/xmos_utils.cmake")

## Define project
project(app_freertos_intertile)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(C ASM)

## Set XMOS application variables
set(APP_HW_TARGET bitstream_src/MIC-ARRAY-1V3.xn)
set(APP_COMPILER_FLAGS
        "-Os"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope")
set(APP_XC_SRCS
        "bitstream_src/bitstream.xc"
    )
set(APP_C_SRCS
        "src/main.c"
        "bitstream_src/bitstream.c"
        "src/intertile_ctrl/intertile_ctrl.c"
        "src/intertile_ctrl/intertile_ctrl_t0.c"
        "src/intertile_ctrl/intertile_ctrl_t1.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "src"
        "bitstream_src"
        "src/intertile_ctrl"
    )
set(APP_DEPENDENT_MODULES
        "lib_soc(>=0.5.0)"
    )

set(USE_FREERTOS_SMP False)

## Add lib_soc devices
set(LIB_SOC_INTERTILE ${FULL})

## Register the application
XMOS_REGISTER_APP()
