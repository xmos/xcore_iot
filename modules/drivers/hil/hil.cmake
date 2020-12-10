cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(HIL_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers/hil")

set(LIB_I2C_DIR "${HIL_DIR}/lib_i2c")
set(LIB_I2S_DIR "${HIL_DIR}/lib_i2s")
set(LIB_MIC_ARRAY_DIR "${HIL_DIR}/lib_mic_array")
set(LIB_SPI_DIR "${HIL_DIR}/lib_spi")

#********************************
# Gather I2C sources
#********************************
set(THIS_LIB LIB_I2C)
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

#**********************
# Set user variables
#**********************
set(HIL_SOURCES
    ${LIB_I2C_SOURCES}
    ${LIB_I2S_SOURCES}
    ${LIB_MIC_ARRAY_SOURCES}
    ${LIB_SPI_SOURCES}
)

set(HIL_INCLUDES
    ${LIB_I2C_INCLUDES}
    ${LIB_I2S_INCLUDES}
    ${LIB_MIC_ARRAY_INCLUDES}
    ${LIB_SPI_INCLUDES}
)
