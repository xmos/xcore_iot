cmake_minimum_required(VERSION 3.20)

project(fatfs_mkimage LANGUAGES C)
set(TARGET_NAME fatfs_mkimage)

# Some source files are in the parent directory
set(FATFS_HOST_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET FATFS_HOST_PATH PARENT_PATH FATFS_HOST_PATH)

# Get the XCore SDK root directory
execute_process(COMMAND git rev-parse --show-toplevel OUTPUT_VARIABLE XCORE_SDK_ROOT)
# remove any crlf
string(STRIP ${XCORE_SDK_ROOT} XCORE_SDK_ROOT)

# Retrieve version number to create directory
file(READ ${XCORE_SDK_ROOT}/settings.json JSON_STRING)
# Get the "version" value from the JSON element
string(JSON VERSION_VAL GET ${JSON_STRING} ${IDX} version)

# Determine OS, set up output dirs
if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    set(FATFS_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
    set(FATFS_INSTALL_DIR "/opt/xmos/SDK/${VERSION_VAL}/bin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    set(FATFS_INSTALL_DIR "$ENV{USERPROFILE}\\.xmos\\SDK\\${VERSION_VAL}\\bin")
endif()

set(APP_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/fatfs_mkimage.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/fatfs_ops.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/directory_add.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/diskio.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ramdisk.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ffsystem.c"
    "${CMAKE_CURRENT_LIST_DIR}/argtable/argtable3.c"
    "${FATFS_HOST_PATH}/thirdparty/src/ff.c"
)

set(APP_INCLUDES
    "."
    "${FATFS_HOST_PATH}/thirdparty/api/"
    "${CMAKE_CURRENT_LIST_DIR}/src/"
    "${CMAKE_CURRENT_LIST_DIR}/"
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES})
install(TARGETS ${TARGET_NAME} DESTINATION ${FATFS_INSTALL_DIR})

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
