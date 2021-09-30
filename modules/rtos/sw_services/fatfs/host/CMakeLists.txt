cmake_minimum_required(VERSION 3.14)

project(avona_control LANGUAGES C)
set(TARGET_NAME fatfs_mkimage)

# Disable in-source build.
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

# Tell cmake where to install things
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install CACHE PATH "" FORCE)

set(APP_SOURCES
    "fatfs_mkimage.c"
    "fatfs_ops.c"
    "directory_add.c"
    "diskio.c"
    "ramdisk.c"
    "ffsystem.c"
    "argtable/argtable3.c"
    "../thirdparty/src/ff.c"
)

set(APP_INCLUDES
    "."
    "../thirdparty/api/"
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES})

if (MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /O2 /W4)
    target_link_options(${TARGET_NAME} PRIVATE "")
elseif (CMAKE_COMPILER_IS_GNUCC)
    message(STATUS "Building with GCC")
    target_compile_options(${TARGET_NAME} PRIVATE -O2 -Wall)
    target_link_options(${TARGET_NAME} PRIVATE "")
else ()
    message(FATAL_ERROR "Unsupported compiler!")
endif ()

