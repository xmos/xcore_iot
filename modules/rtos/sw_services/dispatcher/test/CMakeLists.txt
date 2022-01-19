cmake_minimum_required(VERSION 3.20)

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


# Get path to XCore SDK
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

# Import XMOS RTOS platform configuration.
# Must be done after setting the configuration options.
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")
include("${XCORE_SDK_PATH}/modules/modules.cmake")
include("${XCORE_SDK_PATH}/modules/rtos/rtos.cmake")

project(dispatcher_tests VERSION 1.0.0)

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

add_executable(dispatcher_tests)

target_compile_options(dispatcher_tests PRIVATE ${BUILD_FLAGS})
target_link_options(dispatcher_tests PRIVATE ${BUILD_FLAGS})

set_target_properties(dispatcher_tests PROPERTIES OUTPUT_NAME dispatcher_tests.xe)

#**********************
# targets
#**********************
include("${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake")

target_sources(dispatcher_tests
  PRIVATE ${KERNEL_SOURCES}
  PRIVATE ${RTOS_SUPPORT_SOURCES}
  PRIVATE ${OSAL_SOURCES}
  PRIVATE ${DISPATCHER_SOURCES}
  PRIVATE ${UTILS_SOURCES}
  PRIVATE ${UNITY_SOURCES}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispatch_job.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispatch_group.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_threads_dispatcher.c"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/test_isr_dispatcher.c"
)

target_include_directories(dispatcher_tests
  PRIVATE ${KERNEL_INCLUDES}
  PRIVATE ${RTOS_SUPPORT_INCLUDES}
  PRIVATE ${OSAL_INCLUDES}
  PRIVATE ${DISPATCHER_INCLUDES}
  PRIVATE ${UTILS_INCLUDES}
  PRIVATE ${UNITY_INCLUDES}
  PRIVATE "src"
)

install(TARGETS dispatcher_tests DESTINATION ${INSTALL_DIR})
