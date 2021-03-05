PROJECT_NAME = independent_tiles

BOARD ?= XCORE-AI-EXPLORER

PLATFORM_USES_TILE_0 = true
PLATFORM_USES_TILE_1 = true
XE_BASE_TILE         = 0

BUILD_DIR  = build
OUTPUT_DIR = bin

EXECUTABLE = $(OUTPUT_DIR)/$(PROJECT_NAME).xe

.PHONY: all clean distclean run

all: $(EXECUTABLE)

clean:
	$(MULTITILE_CLEANUP_CMDS)
	rm -f $(EXECUTABLE)

distclean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

run: $(EXECUTABLE)
	xrun --xscope $(EXECUTABLE)

include multitile_build.mk
