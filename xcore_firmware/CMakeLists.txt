cmake_minimum_required(VERSION 3.14)

#**********************
# Options
#**********************
OPTION (X86 "If enabled, will build the x86 version." OFF)

#**********************
# Setup XMOS toolchain
#**********************
if (NOT X86)
  set(CMAKE_SYSTEM_NAME XMOS)
  set(CMAKE_C_COMPILER "xcc")
  set(CMAKE_CXX_COMPILER  "xcc")
  set(CMAKE_ASM_COMPILER  "xcc")
  set(CMAKE_AR "xmosar")
  set(CMAKE_C_COMPILER_AR "xmosar")
  set(CMAKE_CXX_COMPILER_AR "xmosar")
  set(CMAKE_ASM_COMPILER_AR "xmosar")

  set(CMAKE_C_COMPILER_FORCED TRUE)
  set(CMAKE_CXX_COMPILER_FORCED TRUE)
  set(CMAKE_ASM_COMPILER_FORCED TRUE)

  set(CMAKE_C_FLAGS "" CACHE STRING "C Compiler Base Flags" FORCE)
  set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "C++ Compiler Base Flags" FORCE)
  set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
endif ()

set(CMAKE_CXX_FLAGS_INIT "-std=c++11")

enable_language(CXX C ASM)

#**********************
# Disable in-source build.
#**********************
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

project(xcore_interpreter VERSION 1.0.0)

include("${PROJECT_SOURCE_DIR}/../../utils/cmake/xcore_interpreter.cmake")

#**********************
# install
#**********************
set(INSTALL_DIR "${PROJECT_SOURCE_DIR}/bin")

#**********************
# Build flags
#**********************
# Optimization 
# -DNDEBUG                        # define this for remove debug and profiling 
# -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging

if (X86)
  add_executable(xcore_interpreter_host)

  set(BUILD_FLAGS
    "-g"
    "-DTF_LITE_DISABLE_X86_NEON"
    "-O3"
  )
  target_link_libraries(xcore_interpreter_host stdc++ m pthread)

  target_compile_options(xcore_interpreter_host PRIVATE ${BUILD_FLAGS})
else (X86)
  add_executable(xcore_interpreter_xscope)
  add_executable(xcore_interpreter_cmdline)

  set(BUILD_FLAGS
    "-fxscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/config.xscope"
    "-target=XCORE-AI-EXPLORER"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "-Wno-unknown-pragmas"
    "-report"
    "-fno-rtti"
    #"-DNDEBUG" 
    "-DXCORE"
    "-Os"
  )

  target_compile_options(xcore_interpreter_xscope PRIVATE ${BUILD_FLAGS})
  target_compile_options(xcore_interpreter_cmdline PRIVATE ${BUILD_FLAGS} -fcmdline-buffer-bytes=2048)

  target_link_options(xcore_interpreter_xscope PRIVATE ${BUILD_FLAGS} -lquadspi -w)
  target_link_options(xcore_interpreter_cmdline PRIVATE ${BUILD_FLAGS} -lquadspi -w -fcmdline-buffer-bytes=2048)

  set_target_properties(xcore_interpreter_xscope PROPERTIES OUTPUT_NAME xcore_interpreter_xscope.xe)
  set_target_properties(xcore_interpreter_cmdline PROPERTIES OUTPUT_NAME xcore_interpreter_cmdline.xe)
endif (X86)


#**********************
# targets
#**********************
if (X86)
  target_sources(xcore_interpreter_host
    PRIVATE ${XCORE_INTERPRETER_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main_cmdline.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inference_engine.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
  )

  target_include_directories(xcore_interpreter_host
    PRIVATE ${XCORE_INTERPRETER_INCLUDE_DIRS}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  install(TARGETS xcore_interpreter_host DESTINATION ${INSTALL_DIR})
else (X86)
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/main.xc" PROPERTIES LANGUAGE C)

  target_sources(xcore_interpreter_xscope
    PRIVATE ${XCORE_INTERPRETER_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main_xscope.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inference_engine.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.xc"
  )

  target_sources(xcore_interpreter_cmdline
    PRIVATE ${XCORE_INTERPRETER_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main_cmdline.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inference_engine.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ops_resolver.cc"
  )

  target_include_directories(xcore_interpreter_xscope
    PRIVATE ${XCORE_INTERPRETER_INCLUDE_DIRS}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  target_include_directories(xcore_interpreter_cmdline
    PRIVATE ${XCORE_INTERPRETER_INCLUDE_DIRS}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )

  install(TARGETS xcore_interpreter_xscope xcore_interpreter_cmdline DESTINATION ${INSTALL_DIR})
endif (X86)

