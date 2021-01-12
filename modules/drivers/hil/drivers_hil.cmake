cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(HIL_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers/hil")

set(RTOS_I2C_HIL_DIR "${HIL_DIR}/lib_i2c")
set(LIB_I2S_DIR "${HIL_DIR}/lib_i2s")
set(LIB_MIC_ARRAY_DIR "${HIL_DIR}/lib_mic_array")
set(LIB_SPI_DIR "${HIL_DIR}/lib_spi")
set(LIB_QSPI_IO_DIR "${HIL_DIR}/lib_qspi_io")

#**********************
# Options
#**********************
option(USE_RTOS_I2C_HIL "Enable to include I2C HIL" TRUE)
option(USE_RTOS_I2S_HIL "Enable to include I2S HIL" TRUE)
option(USE_RTOS_MIC_ARRAY_HIL "Enable to include microphone array HIL" TRUE)
option(USE_RTOS_SPI_HIL "Enable to include SPI HIL" TRUE)
option(USE_RTOS_QSPI_HIL "Enable to include QSPI HIL" TRUE)

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
endif()
unset(THIS_LIB)

#********************************
# Gather I2S sources
#********************************
set(THIS_LIB LIB_I2S)
set(${THIS_LIB}_FLAGS "-Os")

string(TOLOWER ${THIS_LIB} THIS_PATH)
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

unset(THIS_LIB)

#********************************
# Gather mic_array sources
#********************************
set(THIS_LIB LIB_MIC_ARRAY)
set(${THIS_LIB}_FLAGS "-O3")

string(TOLOWER ${THIS_LIB} THIS_PATH)
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

unset(THIS_LIB)

#********************************
# Gather SPI sources
#********************************
set(THIS_LIB LIB_SPI)
set(${THIS_LIB}_FLAGS "-O3")

string(TOLOWER ${THIS_LIB} THIS_PATH)
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

unset(THIS_LIB)

#********************************
# Gather QSPI I/O sources
#********************************
set(THIS_LIB LIB_QSPI_IO)
set(${THIS_LIB}_FLAGS "-O2")

string(TOLOWER ${THIS_LIB} THIS_PATH)
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

unset(THIS_LIB)

#**********************
# Set user variables
#**********************
set(DRIVERS_HIL_SOURCES
    ${RTOS_I2C_HIL_SOURCES}
    ${LIB_I2S_SOURCES}
    ${LIB_MIC_ARRAY_SOURCES}
    ${LIB_SPI_SOURCES}
    ${LIB_QSPI_IO_SOURCES}
)

set(DRIVERS_HIL_INCLUDES
    ${RTOS_I2C_HIL_INCLUDES}
    ${LIB_I2S_INCLUDES}
    ${LIB_MIC_ARRAY_INCLUDES}
    ${LIB_SPI_INCLUDES}
    ${LIB_QSPI_IO_INCLUDES}
)

list(REMOVE_DUPLICATES DRIVERS_HIL_SOURCES)
list(REMOVE_DUPLICATES DRIVERS_HIL_INCLUDES)
