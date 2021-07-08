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

# Specify configuration
set(MULTITILE_BUILD FALSE)
set(USE_DISPATCHER TRUE)

# Import XMOS RTOS platform configuration.
# Must be done after setting the configuration options.
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")
include("$ENV{XCORE_SDK_PATH}/modules/rtos/rtos.cmake")

project(dispatcher_examples VERSION 1.0.0)

enable_language(CXX C ASM)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")

#**********************
# Build flags
#**********************
set(BUILD_FLAGS
  "-target=XCORE-AI-EXPLORER"
  "-mcmodel=large"
  "-fxscope"
  "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
  "-Wno-xcore-fptrgroup"
  "-Wno-unknown-pragmas"
  "-report"
  "-DDEBUG_PRINT_ENABLE=1"
  "-march=xs3a"
  "-Os"
)

#**********************
# hello_world target
#**********************

add_executable(hello_world)

target_compile_options(hello_world PRIVATE ${BUILD_FLAGS})
target_link_options(hello_world PRIVATE ${BUILD_FLAGS})

set_target_properties(hello_world PROPERTIES OUTPUT_NAME hello_world.xe)

target_sources(hello_world
  PRIVATE ${KERNEL_SOURCES}
  PRIVATE ${RTOS_SUPPORT_SOURCES}
  PRIVATE ${OSAL_SOURCES}
  PRIVATE ${DISPATCHER_SOURCES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world.c"
)

target_include_directories(hello_world
  PRIVATE ${KERNEL_INCLUDES}
  PRIVATE ${RTOS_SUPPORT_INCLUDES}
  PRIVATE ${OSAL_INCLUDES}
  PRIVATE ${DISPATCHER_INCLUDES}
  PRIVATE "src"
)

install(TARGETS hello_world DESTINATION ${INSTALL_DIR})

#***********************
# matrix_multiply target
#***********************

add_executable(matrix_multiply)

target_compile_options(matrix_multiply PRIVATE ${BUILD_FLAGS})
target_link_options(matrix_multiply PRIVATE ${BUILD_FLAGS})

set_target_properties(matrix_multiply PROPERTIES OUTPUT_NAME matrix_multiply.xe)

target_sources(matrix_multiply
  PRIVATE ${KERNEL_SOURCES}
  PRIVATE ${RTOS_SUPPORT_SOURCES}
  PRIVATE ${OSAL_SOURCES}
  PRIVATE ${DISPATCHER_SOURCES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/matrix_multiply.c"
)

target_include_directories(matrix_multiply
  PRIVATE ${KERNEL_INCLUDES}
  PRIVATE ${RTOS_SUPPORT_INCLUDES}
  PRIVATE ${OSAL_INCLUDES}
  PRIVATE ${DISPATCHER_INCLUDES}
  PRIVATE "src"
)

install(TARGETS matrix_multiply DESTINATION ${INSTALL_DIR})
