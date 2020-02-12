set(DEVICE_NAME LIB_SOC_ETHERNET)
set(DEVICE_ADD_COMPILER_FLAGS "")
set(DEVICE_XC_SRCS
        "src/peripherals/bitstream/ethernet_dev/eth_dev.xc")
set(DEVICE_C_SRCS
        "")
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bitstream/ethernet_dev")
set(DEVICE_DEPENDENT_MODULES
        "lib_ethernet(>=3.4.0)"
        "lib_random(>=1.0.0)"
        "lib_otpinfo(>=2.0.1)")
set(DEVICE_OPTIONAL_HEADERS
        "")
