cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(HIL_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers/hil")

set(RTOS_I2C_HIL_DIR "${HIL_DIR}/lib_i2c")
set(RTOS_I2S_HIL_DIR "${HIL_DIR}/lib_i2s")
set(RTOS_MIC_ARRAY_HIL_DIR "${HIL_DIR}/lib_mic_array")
set(RTOS_SPI_HIL_DIR "${HIL_DIR}/lib_spi")
set(RTOS_QSPI_IO_HIL_DIR "${HIL_DIR}/lib_qspi_io")

#**********************
# Options
#**********************
option(USE_RTOS_I2C_HIL "Enable to include I2C HIL" TRUE)
option(USE_RTOS_I2S_HIL "Enable to include I2S HIL" TRUE)
option(USE_RTOS_MIC_ARRAY_HIL "Enable to include microphone array HIL" TRUE)
option(USE_RTOS_SPI_HIL "Enable to include SPI HIL" TRUE)
option(USE_RTOS_QSPI_IO_HIL "Enable to include QSPI HIL" TRUE)

#********************************
# Gather I2C sources
#********************************
set(THIS_LIB RTOS_I2C_HIL)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")
    set(THIS_PATH lib_i2c)

    file(GLOB_RECURSE ${THIS_LIB}_XC_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.xc")
    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.S")

    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_XC_SOURCES}
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${THIS_PATH}/api"
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather I2S sources
#********************************
set(THIS_LIB RTOS_I2S_HIL)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-Os")
    set(THIS_PATH lib_i2s)

    file(GLOB_RECURSE ${THIS_LIB}_XC_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.xc")
    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.S")

    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_XC_SOURCES}
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${THIS_PATH}/api"
    )

    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather mic_array sources
#********************************
set(THIS_LIB RTOS_MIC_ARRAY_HIL)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-O3")
    set(THIS_PATH lib_mic_array)

    file(GLOB_RECURSE ${THIS_LIB}_XC_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.xc")
    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.S")

    list(REMOVE_ITEM ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/fir/make_mic_dual_stage_3_coefs.c")

    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_XC_SOURCES}
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${THIS_PATH}/api"
        "${${THIS_LIB}_DIR}/${THIS_PATH}/src/fir"
    )

    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather QSPI I/O sources
#********************************
set(THIS_LIB RTOS_QSPI_IO_HIL)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-O2")
    set(THIS_PATH lib_qspi_io)

    file(GLOB_RECURSE ${THIS_LIB}_XC_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.xc")
    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.S")

    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_XC_SOURCES}
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${THIS_PATH}/api"
    )

    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather SPI sources
#********************************
set(THIS_LIB RTOS_SPI_HIL)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_FLAGS "-O3")
    set(THIS_PATH lib_spi)

    file(GLOB_RECURSE ${THIS_LIB}_XC_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.xc")
    file(GLOB_RECURSE ${THIS_LIB}_C_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.c")
    file(GLOB_RECURSE ${THIS_LIB}_ASM_SOURCES "${${THIS_LIB}_DIR}/${THIS_PATH}/src/*.S")

    set(${THIS_LIB}_SOURCES
        ${${THIS_LIB}_XC_SOURCES}
        ${${THIS_LIB}_C_SOURCES}
        ${${THIS_LIB}_ASM_SOURCES}
    )

    set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/${THIS_PATH}/api"
    )

    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#**********************
# Set user variables
#**********************
set(DRIVERS_HIL_SOURCES
    ${RTOS_I2C_HIL_SOURCES}
    ${RTOS_I2S_HIL_SOURCES}
    ${RTOS_MIC_ARRAY_HIL_SOURCES}
    ${RTOS_QSPI_IO_HIL_SOURCES}
    ${RTOS_SPI_HIL_SOURCES}
)

set(DRIVERS_HIL_INCLUDES
    ${RTOS_I2C_HIL_INCLUDES}
    ${RTOS_I2S_HIL_INCLUDES}
    ${RTOS_MIC_ARRAY_HIL_INCLUDES}
    ${RTOS_QSPI_IO_HIL_INCLUDES}
    ${RTOS_SPI_HIL_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_HIL_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_HIL_INCLUDES)
