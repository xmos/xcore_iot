set(DEVICE_NAME LIB_SOC_SDRAM)
set(DEVICE_ADD_COMPILER_FLAGS "")
set(DEVICE_XC_SRCS
        "src/peripherals/bitstream/sdram_dev/sdram_dev.xc")
set(DEVICE_CXX_SRCS
        "")
set(DEVICE_C_SRCS
        "")
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bitstream/sdram_dev")
set(DEVICE_DEPENDENT_MODULES
        "lib_sdram(>=3.2.0)")
set(DEVICE_OPTIONAL_HEADERS
        "")
