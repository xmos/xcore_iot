# This variable is used for setting the EXECUTABLE name
# and should be equal to the cmake project name
PROJECT_NAME = person_detect

# BOARD is used by the XMOS RTOS platform .cmake file
BOARD ?= XCORE-AI-EXPLORER
USE_EXTMEM ?= 0
CMAKE_ARGS = -DUSE_EXTMEM=${USE_EXTMEM}

# XE_BASE_TILE specifies the tile build into which other tiles will be merged
# It defaults to 0 if not specified.
XE_BASE_TILE = 0

# BUILD_DIR and OUTPUT_DIR are used by the multitile_build.mk file.
# If not specified they default to 'build' and 'bin' respectively.
BUILD_DIR  = build
OUTPUT_DIR = bin

# EXECUTABLE is used below for recipes
# This value should not be modified
EXECUTABLE = $(OUTPUT_DIR)/$(PROJECT_NAME).xe

.PHONY: all clean distclean run $(EXECUTABLE)

all: $(EXECUTABLE)

clean:
	[ -d "$(BUILD_DIR)" ] && cd $(BUILD_DIR) && make clean

distclean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

run: $(EXECUTABLE)
	xrun --xscope $(EXECUTABLE)

$(EXECUTABLE) :
	cmake -B $(BUILD_DIR) -DBOARD=${BOARD} -DXE_BASE_TILE=$(XE_BASE_TILE) -DOUTPUT_DIR=$(OUTPUT_DIR)
	cd $(BUILD_DIR) && make -j