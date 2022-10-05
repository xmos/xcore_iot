cmake_minimum_required(VERSION 3.20)

project(vcd2psf LANGUAGES C)
set(TARGET_NAME vcd2psf)

set(FATFS_HOST_PATH "${CMAKE_CURRENT_LIST_DIR}")

file(READ ${XCORE_SDK_ROOT}/settings.json JSON_STRING)
# Get the "version" value from the JSON element
string(JSON VERSION_VAL GET ${JSON_STRING} ${IDX} version)

# Determine OS, set up output dirs
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(VCD2PSF_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(VCD2PSF_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(VCD2PSF_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\SDK\\${VERSION_VAL}\\bin")
endif()

set(APP_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/vcd2psf.c"
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES})
install(TARGETS ${TARGET_NAME} DESTINATION ${VCD2PSF_INSTALL_DIR})

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
