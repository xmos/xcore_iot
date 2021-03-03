cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(DRIVERS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/rtos/drivers")

set(OSAL_DIR "${DRIVERS_DIR}/osal")
set(RTOS_GPIO_DRIVER_DIR "${DRIVERS_DIR}/gpio")
set(RTOS_I2C_DRIVER_DIR "${DRIVERS_DIR}/i2c")
set(RTOS_I2S_DRIVER_DIR "${DRIVERS_DIR}/i2s")
set(RTOS_INTERTILE_DRIVER_DIR "${DRIVERS_DIR}/intertile")
set(RTOS_MIC_ARRAY_DRIVER_DIR "${DRIVERS_DIR}/mic_array")
set(RTOS_RPC_DRIVER_DIR "${DRIVERS_DIR}/rpc")
set(RTOS_SPI_DRIVER_DIR "${DRIVERS_DIR}/spi")
set(RTOS_USB_DRIVER_DIR "${DRIVERS_DIR}/usb")
set(RTOS_SWMEM_DRIVER_DIR "${DRIVERS_DIR}/swmem")
set(RTOS_QSPI_FLASH_DRIVER_DIR "${DRIVERS_DIR}/qspi_flash")
set(RTOS_TRACE_DRIVER_DIR "${DRIVERS_DIR}/trace")
set(RTOS_WIFI_DRIVER_DIR "${DRIVERS_DIR}/wifi")

#**********************
# Options
#**********************
option(USE_RTOS_GPIO_DRIVER "Enable to include RTOS GPIO driver" TRUE)
option(USE_RTOS_I2C_DRIVER "Enable to include RTOS I2C driver" TRUE)
option(USE_RTOS_I2S_DRIVER "Enable to include RTOS I2S driver" TRUE)
option(USE_RTOS_INTERTILE_DRIVER "Enable to include RTOS intertile communication driver" TRUE)
option(USE_RTOS_MIC_ARRAY_DRIVER "Enable to include RTOS microphone array driver" TRUE)
option(USE_RTOS_RPC_DRIVER "Enable to include RTOS intertile remote procedure call driver" TRUE)
option(USE_RTOS_SPI_DRIVER "Enable to include RTOS SPI driver" TRUE)
option(USE_RTOS_USB_DRIVER "Enable to include RTOS USB driver" FALSE)
option(USE_RTOS_SWMEM_DRIVER "Enable to include RTOS SWMem driver" TRUE)
option(USE_RTOS_QSPI_FLASH_DRIVER "Enable to include RTOS QSPI flash driver" TRUE)
option(USE_RTOS_TRACE_DRIVER "Enable to include RTOS trace driver" TRUE)
option(USE_RTOS_WIFI_DRIVER "Enable to include RTOS WiFi driver" TRUE)

#********************************
# Gather OSAL sources
#********************************
set(THIS_LIB OSAL)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}"
)

message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
unset(THIS_LIB)

#********************************
# Gather GPIO sources
#********************************
set(THIS_LIB RTOS_GPIO_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather I2C sources
#********************************
set(THIS_LIB RTOS_I2C_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather I2S sources
#********************************
set(THIS_LIB RTOS_I2S_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather intertile sources
#********************************
set(THIS_LIB RTOS_INTERTILE_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather mic array sources
#********************************
set(THIS_LIB RTOS_MIC_ARRAY_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather rpc sources
#********************************
set(THIS_LIB RTOS_RPC_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather SPI sources
#********************************
set(THIS_LIB RTOS_SPI_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather USB sources
#********************************
set(THIS_LIB RTOS_USB_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/*.S")
    
    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather SwMem sources
#********************************
set(THIS_LIB RTOS_SWMEM_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-O3")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather qspi flash sources
#********************************
set(THIS_LIB RTOS_QSPI_FLASH_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather trace sources
#********************************
set(THIS_LIB RTOS_TRACE_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")

    file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather wifi sources
#********************************
set(THIS_LIB RTOS_WIFI_DRIVER)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-USL_WFX_USE_SECURE_LINK")

    if(${RTOS_WIFI_CHIP} STREQUAL "sl_wf200")
        set(${THIS_LIB}_SOURCES
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/sl_wfx_host_spi.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/sl_wfx_host_task.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/sl_wfx_host.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/sl_wfx_iot_wifi.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/sl_wfx_network_interface.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/sl_wfx.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus/sl_wfx_bus_spi.c"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link/sl_wfx_secure_link.c"
        )

        set(${THIS_LIB}_INCLUDES
            "${${THIS_LIB}_DIR}/api"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/bus"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/firmware"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/thirdparty/wfx-fullMAC-driver/wfx_fmac_driver/secure_link"
        )
    else()
        file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}/*.c")

        set(${THIS_LIB}_INCLUDES
            "${${THIS_LIB}_DIR}/api"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}"
            "${${THIS_LIB}_DIR}/${RTOS_WIFI_CHIP}/${RTOS_CMAKE_RTOS}"
        )
    endif()

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#**********************
# set user variables
#**********************
set(DRIVERS_RTOS_SOURCES
    ${OSAL_SOURCES}
    ${RTOS_GPIO_DRIVER_SOURCES}
    ${RTOS_I2C_DRIVER_SOURCES}
    ${RTOS_I2S_DRIVER_SOURCES}
    ${RTOS_INTERTILE_DRIVER_SOURCES}
    ${RTOS_MIC_ARRAY_DRIVER_SOURCES}
    ${RTOS_RPC_DRIVER_SOURCES}
    ${RTOS_SPI_DRIVER_SOURCES}
    ${RTOS_USB_DRIVER_SOURCES}
    ${RTOS_SWMEM_DRIVER_SOURCES}
    ${RTOS_QSPI_FLASH_DRIVER_SOURCES}
    ${RTOS_TRACE_DRIVER_SOURCES}
)

set(DRIVERS_RTOS_INCLUDES
    ${OSAL_INCLUDES}
    ${RTOS_GPIO_DRIVER_INCLUDES}
    ${RTOS_I2C_DRIVER_INCLUDES}
    ${RTOS_I2S_DRIVER_INCLUDES}
    ${RTOS_INTERTILE_DRIVER_INCLUDES}
    ${RTOS_MIC_ARRAY_DRIVER_INCLUDES}
    ${RTOS_RPC_DRIVER_INCLUDES}
    ${RTOS_SPI_DRIVER_INCLUDES}
    ${RTOS_USB_DRIVER_INCLUDES}
    ${RTOS_SWMEM_DRIVER_INCLUDES}
    ${RTOS_QSPI_FLASH_DRIVER_INCLUDES}
    ${RTOS_TRACE_DRIVER_INCLUDES}
    ${DRIVERS_DIR}
)

list(REMOVE_DUPLICATES DRIVERS_RTOS_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_RTOS_INCLUDES)

set(DRIVERS_RTOS_NETWORKING_SOURCES
    ${RTOS_WIFI_DRIVER_SOURCES}
)

set(DRIVERS_RTOS_NETWORKING_INCLUDES
    ${RTOS_WIFI_DRIVER_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_RTOS_NETWORKING_INCLUDES)
