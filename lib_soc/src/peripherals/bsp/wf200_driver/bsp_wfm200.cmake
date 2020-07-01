set(DEVICE_NAME LIB_SOC_WFM200)
set(DEVICE_ADD_COMPILER_FLAGS "-USL_WFX_USE_SECURE_LINK")
set(DEVICE_XC_SRCS
        "")
set(DEVICE_C_SRCS
        "src/peripherals/bsp/wf200_driver/freertos/sl_wfx_host.c"
        "src/peripherals/bsp/wf200_driver/freertos/sl_wfx_host_spi.c"
        "src/peripherals/bsp/wf200_driver/freertos/sl_wfx_host_task.c"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/sl_wfx.c"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus.c"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus_spi.c"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link/sl_wfx_secure_link.c"
        )
set(DEVICE_ASM_SRCS
        "")
set(DEVICE_INCLUDES
        "src/peripherals/bsp/wf200_driver/freertos"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/firmware"
        "src/peripherals/bsp/wf200_driver/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link"
        )
set(DEVICE_DEPENDENT_MODULES
        "")
set(DEVICE_OPTIONAL_HEADERS
        "")
