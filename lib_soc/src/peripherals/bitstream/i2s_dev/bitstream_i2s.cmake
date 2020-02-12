set(DEVICE_NAME LIB_SOC_I2S)
set(DEVICE_ADD_COMPILER_FLAGS "")
set(DEVICE_XC_SRCS
        "src/peripherals/bitstream/i2s_dev/i2s_dev.xc")
set(DEVICE_C_SRCS
        "src/peripherals/bitstream/i2s_dev/fifo.c")
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bitstream/i2s_dev")
set(DEVICE_DEPENDENT_MODULES
        "lib_i2s(>=4.0.0)")
set(DEVICE_OPTIONAL_HEADERS
        "")
