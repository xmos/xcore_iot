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
set(USE_DISPATCH_QUEUE TRUE)

# Import XMOS RTOS platform configuration.
# Must be done after setting the configuration options.
include("$ENV{XMOS_AIOT_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")
include("$ENV{XMOS_AIOT_SDK_PATH}/modules/rtos/rtos.cmake")

project(dispatch_queue_tests VERSION 1.0.0)

enable_language(XC CXX C ASM)

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")

#**********************
# Build flags
#**********************
set(BUILD_FLAGS
  "-target=XCORE-AI-EXPLORER"
  "-fcmdline-buffer-bytes=1024"
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

add_executable(dispatch_queue_tests)

target_compile_options(dispatch_queue_tests PRIVATE ${BUILD_FLAGS})
target_link_options(dispatch_queue_tests PRIVATE ${BUILD_FLAGS})

set_target_properties(dispatch_queue_tests PROPERTIES OUTPUT_NAME dispatch_queue_tests.xe)

#**********************
# targets
#**********************
include("${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake")

target_sources(dispatch_queue_tests
  PRIVATE ${KERNEL_SOURCES}
  PRIVATE ${RTOS_SUPPORT_SOURCES}
  PRIVATE ${OSAL_SOURCES}
  PRIVATE ${DISPATCH_QUEUE_SOURCES}
  PRIVATE ${UNITY_SOURCES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispatch_task.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispatch_group.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispatch_queue.c"
)

target_include_directories(dispatch_queue_tests
  PRIVATE ${KERNEL_INCLUDES}
  PRIVATE ${RTOS_SUPPORT_INCLUDES}
  PRIVATE ${OSAL_INCLUDES}
  PRIVATE ${DISPATCH_QUEUE_INCLUDES}
  PRIVATE ${UNITY_INCLUDES}
  PRIVATE "src"
)

install(TARGETS dispatch_queue_tests DESTINATION ${INSTALL_DIR})
