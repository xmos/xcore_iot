# This variable is used for setting the EXECUTABLE name
# and should be equal to the cmake project name
PROJECT_NAME = explorer_board

# BUILD_DIR and OUTPUT_DIR are used by the multitile_build.cmake file.
# If not specified they default to 'build' and 'bin' respectively.
BUILD_DIR  = build
OUTPUT_DIR = bin

# EXECUTABLE is used below for recipes
# This value should not be modified
EXECUTABLE = $(OUTPUT_DIR)/$(PROJECT_NAME).xe

.PHONY: all clean distclean run

all: $(EXECUTABLE)

clean:
	[ -d "$(BUILD_DIR)" ] && cd $(BUILD_DIR) && make clean

distclean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

run: $(EXECUTABLE)
	xrun --xscope $(EXECUTABLE)

$(EXECUTABLE) :
	cmake -B $(BUILD_DIR) -DOUTPUT_DIR=$(OUTPUT_DIR)
	cd $(BUILD_DIR) && make -j && make install
