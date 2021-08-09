set(CMAKE_SYSTEM_NAME XMOS)

# CMake versions 3.20 and newer now require the ASM dialect to be specified
set(ASM_DIALECT "")

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

set(CMAKE_RANLIB "")
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_FORCED TRUE)

set(CMAKE_C_FLAGS "" CACHE STRING "C Compiler Base Flags" FORCE)
set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "C++ Compiler Base Flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "" FORCE)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/xc_override.cmake")
