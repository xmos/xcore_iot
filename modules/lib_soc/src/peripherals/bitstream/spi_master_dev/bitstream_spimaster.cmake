set(DEVICE_NAME LIB_SOC_SPIMASTER)
set(DEVICE_ADD_COMPILER_FLAGS "")
set(DEVICE_XC_SRCS
        "src/peripherals/bitstream/spi_master_dev/spi_master_dev.xc"
        "src/peripherals/bitstream/spi_master_dev/spi_fast/spi_fast.xc")
set(DEVICE_CXX_SRCS
        "")
set(DEVICE_C_SRCS
        "src/peripherals/bitstream/spi_master_dev/spi_fast/spi_fast.S")
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bitstream/spi_master_dev"
        "src/peripherals/bitstream/spi_master_dev/spi_fast")
set(DEVICE_DEPENDENT_MODULES
        "")
set(DEVICE_OPTIONAL_HEADERS
        "")
