cmake_minimum_required(VERSION 3.14)

#**********************
# Disable in-source build.
#**********************
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

#**********************
# Setup project
#**********************
set(BOARD XCORE-AI-EXPLORER)
set(SDK_PATH $ENV{XCORE_SDK_PATH})

# Specify configuration
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL TRUE)
set(USE_RTOS_GPIO_DRIVER FALSE)
set(USE_RTOS_I2C_DRIVER FALSE)
set(USE_RTOS_I2S_DRIVER FALSE)
set(USE_RTOS_SPI_DRIVER FALSE)
set(USE_RTOS_INTERTILE_DRIVER TRUE)
set(USE_RTOS_MIC_ARRAY_DRIVER FALSE)
set(USE_RTOS_RPC_DRIVER TRUE)
set(USE_RTOS_QSPI_FLASH_DRIVER TRUE)
set(USE_RTOS_WIFI_DRIVER FALSE)
set(USE_RTOS_TRACE_DRIVER FALSE)
set(USE_MULTITILE_SUPPORT FALSE)
set(USE_LIB_DSP FALSE)
set(USE_LIB_LOGGING FALSE)
set(USE_LIB_RANDOM FALSE)
set(USE_LEGACY_COMPAT TRUE)
set(USE_AIF TRUE)
set(USE_DEVICE_MEMORY_SUPPORT TRUE)
set(USE_DISPATCH_QUEUE TRUE)

# Import XMOS RTOS platform configuration.
# Must be done after setting the configuration options.
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_rtos_platform.cmake")

project(model_runner VERSION 1.0.0)

enable_language(CXX C ASM)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")

#**********************
# Build flags
#**********************
set(BUILD_FLAGS
  "-fxscope"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
  "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
  "-mcmodel=large"
  "-report"
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-fno-rtti"
  "-fno-exceptions"
  "-DXCORE"
  "-DTF_LITE_STATIC_MEMORY"
  "-DDEBUG_PRINT_ENABLE=1"
  "-Os"
)

set(CMAKE_CXX_FLAGS  "-std=c++11")

if (${USE_SWMEM})
  add_definitions(-DUSE_SWMEM)
elseif (${USE_EXTMEM})
  add_definitions(-DUSE_EXTMEM)
endif ()

add_executable(model_runner)

target_compile_options(model_runner PRIVATE ${BUILD_FLAGS})

target_link_options(model_runner PRIVATE ${BUILD_FLAGS} -lquadspi -w)

set_target_properties(model_runner PROPERTIES OUTPUT_NAME model_runner.xe)

#**********************
# targets
#**********************

target_sources(model_runner
  PRIVATE ${XMOS_RTOS_PLATFORM_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_model_data.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/test_model_runner.cc"
)

target_include_directories(model_runner
  PRIVATE ${XMOS_RTOS_PLATFORM_INCLUDES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

install(TARGETS model_runner DESTINATION ${INSTALL_DIR})
