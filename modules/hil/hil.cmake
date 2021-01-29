cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(HIL_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/hil")

set(I2C_HIL_DIR "${HIL_DIR}/lib_i2c")
set(I2S_HIL_DIR "${HIL_DIR}/lib_i2s")
set(MIC_ARRAY_HIL_DIR "${HIL_DIR}/lib_mic_array")
set(SPI_HIL_DIR "${HIL_DIR}/lib_spi")
set(QSPI_IO_HIL_DIR "${HIL_DIR}/lib_qspi_io")

#**********************
# Options
#**********************
option(USE_I2C_HIL "Enable to include I2C HIL" TRUE)
option(USE_I2S_HIL "Enable to include I2S HIL" TRUE)
option(USE_MIC_ARRAY_HIL "Enable to include microphone array HIL" TRUE)
option(USE_SPI_HIL "Enable to include SPI HIL" TRUE)
option(USE_QSPI_IO_HIL "Enable to include QSPI HIL" TRUE)

#********************************
# Gather I2C sources
#********************************
set(THIS_LIB I2C_HIL)
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
set(THIS_LIB I2S_HIL)
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
set(THIS_LIB MIC_ARRAY_HIL)
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
set(THIS_LIB QSPI_IO_HIL)
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
set(THIS_LIB SPI_HIL)
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
set(HIL_SOURCES
    ${I2C_HIL_SOURCES}
    ${I2S_HIL_SOURCES}
    ${MIC_ARRAY_HIL_SOURCES}
    ${QSPI_IO_HIL_SOURCES}
    ${SPI_HIL_SOURCES}
)

set(HIL_INCLUDES
    ${I2C_HIL_INCLUDES}
    ${I2S_HIL_INCLUDES}
    ${MIC_ARRAY_HIL_INCLUDES}
    ${QSPI_IO_HIL_INCLUDES}
    ${SPI_HIL_INCLUDES}
)

list(REMOVE_DUPLICATES HIL_SOURCES)
list(REMOVE_DUPLICATES HIL_INCLUDES)
