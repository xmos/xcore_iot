
TARGET = MIC-ARRAY-1V3

APP_NAME = 

SOURCE_DIRS = bitstream_src src src/afr_wifi

INCLUDE_DIRS = $(SOURCE_DIRS)

XCC_FLAGS = -Os -g -report -fxscope -lquadflash -Wxcore-fptrgroup -DDEBUG_PRINT_ENABLE=1 -DSL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_INIT\)

USED_MODULES = FreeRTOS(>=10.2.1) FreeRTOS-Plus(>=10.2.1) lib_soc(>=0.5.0)

VERBOSE = 0

#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.
XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
