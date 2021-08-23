cmake_minimum_required(VERSION 3.14)

## Define project
project(manual_stitch VERSION 0.1)

## Enable languages for project
enable_language(CXX C ASM)

# Directions
# To run, use these commands:
# mkdir build, cd build
# cmake . ../
# cmake --build . --target <custom_target>


## variables
# tile - TODO: wildcard this for iterations
set(TILE0 tile0)
set(TILE1 tile1)
set(XE_FILE explorer_board.xe)


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



## deleteme test command firing
add_custom_target(ccmd_split
    COMMAND
        ${CMAKE_COMMAND} -E echo this might work for something later
	VERBATIM
	# test stuff
	COMMAND echo make temporary tile0 directory
    COMMAND mkdir -p ${TILE0_TEMP_DIR}
	COMMAND echo call xobjdump on tile0 executable, dump files in temp dir
	# syntax: xobjdump --split --split-dir <dir_to_dump_files> <.xe_file>
	COMMAND echo TILE0_SRC_DIR is ${TILE0_SRC_DIR}
	COMMAND xobjdump --split --split-dir ${TILE0_TEMP_DIR} ${TILE0_SRC_DIR}/${XE_FILE}
	
	# do the same for tile1 TODO: iterate
	COMMAND echo make temporary tile1 directory
    COMMAND mkdir -p ${TILE1_TEMP_DIR}
	COMMAND echo call xobjdump on tile1 executable, dump files in temp dir
	# syntax: xobjdump --split --split-dir <dir_to_dump_files> <.xe_file>
	COMMAND xobjdump --split --split-dir ${TILE1_TEMP_DIR} ${TILE1_SRC_DIR}/${XE_FILE}
)

add_custom_target(ccmd_clr
	COMMAND echo clearing the temp dirs
	# TODO: figure out if this sb try/catch or similar
	COMMAND rm -rf ${TEMP}
)

# execute_process(COMMAND mkdir foo)

## TODO: Loop this: TODO: Split tile XE into ELF files
## TODO: stitch into one XE


# ## deleteme Makefile code to translate:
# # Splits up a tile's XE file into its individual ELF files.
# SPLIT_XE = mkdir -p $(BUILD_DIR)/tmp/tile$(1); xobjdump --split --split-dir $(BUILD_DIR)/tmp/tile$(1) $(call TILE_EXECUTABLE,$(1));

# # Replaces the specified tile's ELF file with the correct one from the tile specific build.
# REPLACE_ELF = xobjdump $(EXECUTABLE) -r 0,$(1),$(BUILD_DIR)/tmp/tile$(1)/image_n0c$(1)_2.elf;

# MERGE_XE = $(call SPLIT_XE,$(1)) $(call REPLACE_ELF,$(1))

# $(EXECUTABLE): $(TILE_DEPS)
    # cp $(call TILE_EXECUTABLE,$(XE_BASE_TILE)) $(EXECUTABLE)
    # $(call PER_MERGE_TILE,MERGE_XE)
