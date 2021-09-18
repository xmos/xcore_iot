cmake_minimum_required(VERSION 3.14)

## Define project
project(manual_stitch VERSION 0.1)

## Enable languages for project
enable_language(CXX C ASM)

# Directions
# To build, use these commands in example directory:
# mkdir build, cd build
# cmake . ../
# cmake --build . --target <custom_target>
# e.g.: cmake --build . --target ccmd_split

# cmake --build ./


## variables
# tile - TODO: wildcard this for iterations
set(TILE0 tile0)
set(TILE1 tile1)
# TODO: project_name should be passed in
set(PROJECT_NAME explorer_board)
# Note: This is not the same as EXECUTABLE from multitile_build.mk. This is
# only a placeholder for the project name at this time.
set(EXECUTABLE ${PROJECT_NAME}.xe)
# TODO: TILE_EXECUTABLE can be set when using wildcard substitution


# "dir" abbreviates "directory"
# CMAKE_BINARY_DIR is build/
set(BUILD build)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${BUILD})
set(TEMP tmp)
set(TEMP_DIR ${BUILD_DIR}/${TEMP})
# dir from which to retrieve tile .xe
# TODO: hard code tile xe files' origin unless there's a way to seek them
#		for now, these are manually copied from bin/ to here:
set(TILE0_SRC_DIR ${CMAKE_SOURCE_DIR}/${TILE0})
# dir to deposit xobjdump output
set(TILE0_TEMP_DIR ${TEMP_DIR}/${TILE0})

# do the same for tile1 TODO: iterate
set(TILE1_SRC_DIR ${CMAKE_SOURCE_DIR}/${TILE1})
set(TILE1_TEMP_DIR ${TEMP_DIR}/${TILE1})

# TODO: multitile_build.cmake should be in the cmake tools dir
# include(${XCORE_SDK_PATH}/tools/cmake_utils/multitile_build.cmake)
include(multitile_build.cmake)


# this is another way to use COMMAND, could be useful
# execute_process(COMMAND mkdir foo)


