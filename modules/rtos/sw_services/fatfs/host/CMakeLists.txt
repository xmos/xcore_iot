cmake_minimum_required(VERSION 3.20)

project(fatfs_mkimage LANGUAGES C)
set(TARGET_NAME fatfs_mkimage)

# Disable in-source build.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

# Tell cmake where to install things
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE PATH "" FORCE)

# Some source files are in the parent directory
set(FATFS_HOST_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET FATFS_HOST_PATH PARENT_PATH FATFS_HOST_PATH)
message(STATUS "Using src files at ${FATFS_HOST_PATH}")

# Output application to the parent directory
set(FATFS_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR})
cmake_path(GET FATFS_INSTALL_DIR PARENT_PATH FATFS_INSTALL_DIR)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FATFS_INSTALL_DIR})

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
