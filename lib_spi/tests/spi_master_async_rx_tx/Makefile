# The TARGET variable determines what target system the application is
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling
TARGET = SLICEKIT-A16

# The APP_NAME variable determines the name of the final .xe file. It should
# not include the .xe postfix. If left blank the name will default to
# the project name
APP_NAME = spi_master_async_rx_tx

# The USED_MODULES variable lists other module used by the application.
USED_MODULES = lib_spi lib_spi_master_tester

# The flags passed to xcc when building the application
# You can also set the following to override flags for a particular language:
# XCC_XC_FLAGS, XCC_C_FLAGS, XCC_ASM_FLAGS, XCC_CPP_FLAGS
# If the variable XCC_MAP_FLAGS is set it overrides the flags passed to
# xcc for the final link (mapping) stage.
COMMON_FLAGS = -O2 -g -Wno-reinterpret-alignment

XCC_FLAGS_2010 = $(COMMON_FLAGS) -DBURNT_THREADS=2 -DMOSI_ENABLED=0 -DSPEED_TESTS=1 -DCOMBINED=0
XCC_FLAGS_2110 = $(COMMON_FLAGS) -DBURNT_THREADS=2 -DMOSI_ENABLED=1 -DSPEED_TESTS=1 -DCOMBINED=0
XCC_FLAGS_6010 = $(COMMON_FLAGS) -DBURNT_THREADS=6 -DMOSI_ENABLED=0 -DSPEED_TESTS=1 -DCOMBINED=0
XCC_FLAGS_6110 = $(COMMON_FLAGS) -DBURNT_THREADS=6 -DMOSI_ENABLED=1 -DSPEED_TESTS=1 -DCOMBINED=0

XCC_FLAGS_2030 = $(COMMON_FLAGS) -DBURNT_THREADS=2 -DMOSI_ENABLED=0 -DSPEED_TESTS=3 -DCOMBINED=0
XCC_FLAGS_2130 = $(COMMON_FLAGS) -DBURNT_THREADS=2 -DMOSI_ENABLED=1 -DSPEED_TESTS=3 -DCOMBINED=0
XCC_FLAGS_6030 = $(COMMON_FLAGS) -DBURNT_THREADS=6 -DMOSI_ENABLED=0 -DSPEED_TESTS=3 -DCOMBINED=0
XCC_FLAGS_6130 = $(COMMON_FLAGS) -DBURNT_THREADS=6 -DMOSI_ENABLED=1 -DSPEED_TESTS=3 -DCOMBINED=0

XCC_FLAGS_3011 = $(COMMON_FLAGS) -DBURNT_THREADS=3 -DMOSI_ENABLED=0 -DSPEED_TESTS=1 -DCOMBINED=1
XCC_FLAGS_3111 = $(COMMON_FLAGS) -DBURNT_THREADS=3 -DMOSI_ENABLED=1 -DSPEED_TESTS=1 -DCOMBINED=1
XCC_FLAGS_7011 = $(COMMON_FLAGS) -DBURNT_THREADS=7 -DMOSI_ENABLED=0 -DSPEED_TESTS=1 -DCOMBINED=1
XCC_FLAGS_7111 = $(COMMON_FLAGS) -DBURNT_THREADS=7 -DMOSI_ENABLED=1 -DSPEED_TESTS=1 -DCOMBINED=1

XCC_FLAGS_3031 = $(COMMON_FLAGS) -DBURNT_THREADS=3 -DMOSI_ENABLED=0 -DSPEED_TESTS=3 -DCOMBINED=1
XCC_FLAGS_3131 = $(COMMON_FLAGS) -DBURNT_THREADS=3 -DMOSI_ENABLED=1 -DSPEED_TESTS=3 -DCOMBINED=1
XCC_FLAGS_7031 = $(COMMON_FLAGS) -DBURNT_THREADS=7 -DMOSI_ENABLED=0 -DSPEED_TESTS=3 -DCOMBINED=1
XCC_FLAGS_7131 = $(COMMON_FLAGS) -DBURNT_THREADS=7 -DMOSI_ENABLED=1 -DSPEED_TESTS=3 -DCOMBINED=1

# The XCORE_ARM_PROJECT variable, if set to 1, configures this
# project to create both xCORE and ARM binaries.
XCORE_ARM_PROJECT = 0

# The VERBOSE variable, if set to 1, enables verbose output from the make system.
VERBOSE = 0

XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
