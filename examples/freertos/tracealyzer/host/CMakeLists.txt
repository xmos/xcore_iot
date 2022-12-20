cmake_minimum_required(VERSION 3.20)

# Compile for x86_64 on Mac as we can't support the M1 ARM architecture yet
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "")

project(xscope2psf LANGUAGES C)
set(TARGET_NAME xscope2psf)

set(FATFS_HOST_PATH "${CMAKE_CURRENT_LIST_DIR}")

file(READ ${XCORE_SDK_ROOT}/settings.json JSON_STRING)
# Get the "version" value from the JSON element
string(JSON VERSION_VAL GET ${JSON_STRING} ${IDX} version)

# Determine OS, set up output dirs
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(XSCOPE2PSF_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(XSCOPE2PSF_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(XSCOPE2PSF_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\SDK\\${VERSION_VAL}\\bin")
endif()

set(APP_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/xscope2psf.c"
)

set(APP_INCLUDES
    "$ENV{XMOS_TOOL_PATH}/include/"
)

find_library(XSCOPE_ENDPOINT_LIB NAMES xscope_endpoint.so xscope_endpoint.lib
                                 PATHS $ENV{XMOS_TOOL_PATH}/lib)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES})
target_link_libraries(${TARGET_NAME} PRIVATE ${XSCOPE_ENDPOINT_LIB})
install(TARGETS ${TARGET_NAME} DESTINATION ${XSCOPE2PSF_INSTALL_DIR})

if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "AppleClang"))
    message(STATUS "Configuring for Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Configuring for GCC")
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Configuring for MSVC")
    target_compile_options(${TARGET_NAME} PRIVATE /W3)
    target_link_options(${TARGET_NAME} PRIVATE "")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
else ()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()
