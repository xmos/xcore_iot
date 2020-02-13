set(DEVICE_NAME LIB_SOC_I2C)
set(DEVICE_ADD_COMPILER_FLAGS "")
set(DEVICE_XC_SRCS
        "src/peripherals/bitstream/i2c_dev/i2c_dev.xc")
set(DEVICE_C_SRCS
        "")
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bitstream/i2c_dev")
set(DEVICE_DEPENDENT_MODULES
        "lib_i2c(>=6.0.0)")
set(DEVICE_OPTIONAL_HEADERS
        "")
