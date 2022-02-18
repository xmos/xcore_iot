
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    set(WIFI_CHIP sl_wf200)

    ## Create library target
    add_library(xcore_sdk_rtos_drivers_wifi INTERFACE)
    target_sources(xcore_sdk_rtos_drivers_wifi
        INTERFACE
            ${WIFI_CHIP}/FreeRTOS/sl_wfx_host_spi.c
            ${WIFI_CHIP}/FreeRTOS/sl_wfx_host_task.c
            ${WIFI_CHIP}/FreeRTOS/sl_wfx_host.c
            ${WIFI_CHIP}/FreeRTOS/sl_wfx_iot_wifi.c
            ${WIFI_CHIP}/FreeRTOS/sl_wfx_network_interface.c
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/sl_wfx.c
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus.c
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus_spi.c
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link/sl_wfx_secure_link.c

    )
    target_include_directories(xcore_sdk_rtos_drivers_wifi
        INTERFACE
            api
            ${WIFI_CHIP}
            ${WIFI_CHIP}/FreeRTOS
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/firmware
            ${WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link

    )
    target_compile_options(xcore_sdk_rtos_drivers_wifi
        INTERFACE
            "-USL_WFX_USE_SECURE_LINK"
    )
    target_link_libraries(xcore_sdk_rtos_drivers_wifi
        INTERFACE
            sdk::rtos::osal
            sdk::rtos::FreeRTOS-Plus-TCP
            sdk::rtos::drivers::gpio
            sdk::rtos::drivers::spi
    )

    ## Create an alias
    add_library(sdk::rtos::drivers::wifi ALIAS xcore_sdk_rtos_drivers_wifi)
endif()
