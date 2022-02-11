set(CMAKE_SYSTEM_NAME XCORE_XS3A)

# CMake versions 3.20 and newer now require the ASM dialect to be specified
set(ASM_DIALECT "")

# XMOS_TOOLS_PATH can be used to build with multiple versions of the XTC Tools
#  This should not be confused with XMOS_TOOL_PATH (no "S") which is set in the
#  XTC Tools environment and points to the tools version installed.
if(DEFINED XMOS_TOOLS_PATH)
    set(CMAKE_C_COMPILER "${XMOS_TOOLS_PATH}/xcc")
    set(CMAKE_CXX_COMPILER  "${XMOS_TOOLS_PATH}/xcc")
    set(CMAKE_ASM_COMPILER  "${XMOS_TOOLS_PATH}/xcc")
    set(CMAKE_AR "${XMOS_TOOLS_PATH}/xmosar" CACHE FILEPATH "Archiver")
    set(CMAKE_C_COMPILER_AR "${XMOS_TOOLS_PATH}/xmosar")
    set(CMAKE_CXX_COMPILER_AR "${XMOS_TOOLS_PATH}/xmosar")
    set(CMAKE_ASM_COMPILER_AR "${XMOS_TOOLS_PATH}/xmosar")
else()
    message(STATUS "XMOS_TOOLS_PATH not specified.  CMake will assume tools have been added to PATH.")
    set(CMAKE_C_COMPILER "xcc")
    set(CMAKE_CXX_COMPILER  "xcc")
    set(CMAKE_ASM_COMPILER  "xcc")
    set(CMAKE_AR "xmosar" CACHE FILEPATH "Archiver")
    set(CMAKE_C_COMPILER_AR "xmosar")
    set(CMAKE_CXX_COMPILER_AR "xmosar")
    set(CMAKE_ASM_COMPILER_AR "xmosar")
endif()

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
    SET(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")

    SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)
    SET(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

    SET(CMAKE_ASM_USE_RESPONSE_FILE_FOR_OBJECTS 1)
    SET(CMAKE_ASM_USE_RESPONSE_FILE_FOR_INCLUDES 1)
    SET(CMAKE_ASM_RESPONSE_FILE_LINK_FLAG "@")
endif()

set(CMAKE_RANLIB "")
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)

set(CMAKE_C_FLAGS "-march=xs3a" CACHE STRING "C Compiler Base Flags" FORCE)
set(CMAKE_CXX_FLAGS "-march=xs3a -std=c++11" CACHE STRING "C++ Compiler Base Flags" FORCE)
set(CMAKE_ASM_FLAGS "-march=xs3a" CACHE STRING "ASM Compiler Base Flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)
set(CMAKE_EXECUTABLE_SUFFIX_C   .xe CACHE INTERNAL "" FORCE)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .xe CACHE INTERNAL "" FORCE)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .xe CACHE INTERNAL "" FORCE)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/xc_override.cmake")
