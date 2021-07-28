# Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1
#
# This helper makefile creates a recipe for a multitile XCORE application's .xe binary,
# where the .elf for each tile is independently built and then merged into a single
# .xe file. This allows for conditional compilation per tile by checking the
# THIS_XCORE_TILE preprocessor macro.
#
# This requires that the application have a single CMakeLists.txt file that defines the
# project and that each tile be built from the same body of source code.
# cmake will be run once per tile into separate build directories, and each of these
# cmake projects will then be built. The resulting .xe files from each are combined into
# a single .xe at the output location specified by EXECUTABLE.
#
# TODO: Usage instructions. See example usage in the independent_tiles example app for now.
#

PLATFORM_USES_TILE_0 ?= 1
PLATFORM_USES_TILE_1 ?= 0
PLATFORM_USES_TILE_2 ?= 0
PLATFORM_USES_TILE_3 ?= 0
XE_BASE_TILE ?= 0

BUILD_DIR ?= build
OUTPUT_DIR ?= bin

TILE_BUILD_DIR = $(BUILD_DIR)/tile$(1)
TILE_OUTPUT_DIR = $(OUTPUT_DIR)/tile$(1)

# The CMake generated Makefile per tile.
TILE_MAKE = $(call TILE_BUILD_DIR,$(1))/Makefile

# The XE file per tile. These all get merged into one ultimate XE file.
TILE_EXECUTABLE = $(call TILE_OUTPUT_DIR,$(1))/$(PROJECT_NAME).xe

# Simply add to this list to support more than 4 tiles
TILES = 0 1 2 3

IS_USED_TILE = $(filter 1 true,$(PLATFORM_USES_TILE_$(1)))
IS_NOT_BASE_TILE = $(filter-out $(1),$(XE_BASE_TILE))

# This PER_USED_TILE function runs the provided function once for each
# tile that is used with the tile number as its argument, concatenating
# the result of each together.
PER_USED_TILE = $(foreach tile,$(TILES),$(if $(call IS_USED_TILE,$(tile)),$(call $(1),$(tile))))

# This PER_MERGE_TILE function runs the provided function once for each
# tile that is used and is not the base tile with the tile number as its argument,
# concatenating the result of each together.
PER_MERGE_TILE = $(foreach tile,$(TILES),$(if $(call IS_USED_TILE,$(tile)),$(if $(call IS_NOT_BASE_TILE,$(tile)),$(call $(1),$(tile)))))

# This creates a list of the individual tile XE files.
# These are used as the dependencies of the ultimate combined executable.
TILE_DEPS = $(call PER_USED_TILE,TILE_EXECUTABLE)

# This creates a list of the make commands to clean the build for each tile.
TILE_CLEAN_CMD = make -C $(call TILE_BUILD_DIR,$(1)) clean;
TILE_CLEAN_CMDS = $(call PER_USED_TILE,TILE_CLEAN_CMD)

# Creates a list of cleanup commands that the application's Makefile
# can run for its clean target.
MULTITILE_CLEANUP_CMDS = rm -rf $(BUILD_DIR)/tmp $(TILE_DEPS); $(TILE_CLEAN_CMDS)

.PHONY: .FORCE
.FORCE:

# This ensures that the intermediate Makefile generated for
# each tile is not deleted upon completion.
.PRECIOUS: $(call TILE_MAKE,%)

# Splits up a tile's XE file into its individual ELF files.
SPLIT_XE = mkdir -p $(BUILD_DIR)/tmp/tile$(1); xobjdump --split --split-dir $(BUILD_DIR)/tmp/tile$(1) $(call TILE_EXECUTABLE,$(1));

# Replaces the specified tile's ELF file with the correct one from the tile specific build.
REPLACE_ELF = xobjdump $(EXECUTABLE) -r 0,$(1),$(BUILD_DIR)/tmp/tile$(1)/image_n0c$(1)_2.elf;

MERGE_XE = $(call SPLIT_XE,$(1)) $(call REPLACE_ELF,$(1))

$(EXECUTABLE): $(TILE_DEPS)
	cp $(call TILE_EXECUTABLE,$(XE_BASE_TILE)) $(EXECUTABLE)
	$(call PER_MERGE_TILE,MERGE_XE)

# Creates each tile's Makefile by running CMake for a specific tile.
$(call TILE_MAKE,%):
	cmake -B $(call TILE_BUILD_DIR,$*) -DPROJECT_NAME=$(PROJECT_NAME) -DMULTITILE_BUILD=1 -DTHIS_XCORE_TILE=$* -DBOARD=$(BOARD) -DOUTPUT_DIR=$(OUTPUT_DIR) -DPLATFORM_USES_TILE_0=$(PLATFORM_USES_TILE_0) -DPLATFORM_USES_TILE_1=$(PLATFORM_USES_TILE_1)  -DPLATFORM_USES_TILE_2=$(PLATFORM_USES_TILE_2) -DPLATFORM_USES_TILE_3=$(PLATFORM_USES_TILE_3) $(CMAKE_ARGS)

# Creates each tile's XE file by running make in the specific tile's build directory.
$(call TILE_EXECUTABLE,%): $(call TILE_MAKE,%) .FORCE
	make -C $(call TILE_BUILD_DIR,$*)
