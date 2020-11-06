cmake_minimum_required(VERSION 3.14)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Import XMOS configurations
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_utils.cmake")
set(XMOS_APP_MODULES_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

## Define project
project(person_detection)
set(PROJECT_VERSION 0.5.0)

## Enable languages for project
enable_language(CXX XC C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
        "-DOUTPUT_IMAGE_STREAM"
        "-Os"
        "-g"
        "-report"
        "-fxscope"
        "-Wxcore-fptrgroup"
        "-DDEBUG_PRINT_ENABLE=1"
        "-Wno-unknown-pragmas"
        "-DTF_LITE_STATIC_MEMORY"
        "-D_XCORE_HAS_SWMEM"
        "-DTF_LITE_STRIP_ERROR_STRINGS"
        "-DNDEBUG"
        "-mcmodel=large"
        "-DXCORE"
        "${CMAKE_CURRENT_SOURCE_DIR}/app/config.xscope")
set(APP_XC_SRCS
        ""
    )
set(APP_CXX_SRCS
        ""
    )
set(APP_C_SRCS
        "app/main.c"
        "app/ov2640/ov2640.c"
        "app/ov2640/ov2640_progs.c"
        "app/spi_camera/spi_camera.c"
        "app/queue_to_ai/queue_to_ai.c"
    )
set(APP_ASM_SRCS
        ""
    )
set(APP_INCLUDES
        "app"
        "app/ov2640"
        "app/spi_camera"
        "app/queue_to_ai"
    )
set(APP_DEPENDENT_MODULES
        "lib_soc(>=0.6.0)"
        "inference_engine(>=0.0.0)"
    )

if(${USE_EXTMEM})
  add_definitions(-DUSE_EXTMEM)
endif()

## Set any additional flags only for C++
set(CMAKE_CXX_FLAGS  "-std=c++11")

set(USE_FREERTOS_SMP True)

## Add lib_soc devices
set(LIB_SOC_GPIO ${FULL})
set(LIB_SOC_I2C ${FULL})
set(LIB_SOC_SPIMASTER ${FULL})
set(LIB_SOC_AI ${FULL})

## Register the application
XMOS_REGISTER_APP()
