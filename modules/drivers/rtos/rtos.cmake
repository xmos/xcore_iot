cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(RTOS_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/drivers/rtos")

set(GPIO_DIR "${RTOS_DIR}/gpio")
set(I2C_DIR "${RTOS_DIR}/i2c")
set(I2S_DIR "${RTOS_DIR}/i2s")
set(INTERTILE_DIR "${RTOS_DIR}/intertile")
set(MIC_ARRAY_DIR "${RTOS_DIR}/mic_array")
set(RPC_DIR "${RTOS_DIR}/rpc")
set(SPI_DIR "${RTOS_DIR}/spi")
set(TRACE_DIR "${RTOS_DIR}/trace")

set(RTOS_CMAKE_RTOS "FreeRTOS") # Only FreeRTOS is currently supported

#********************************
# Gather kernel sources
#********************************
include("$ENV{XMOS_AIOT_SDK_PATH}/modules/rtos/${RTOS_CMAKE_RTOS}/kernel.cmake")

#********************************
# Gather GPIO sources
#********************************
set(THIS_LIB GPIO)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
)

unset(THIS_LIB)

#********************************
# Gather I2C sources
#********************************
set(THIS_LIB I2C)
set(${THIS_LIB}_FLAGS "-O3")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}" # TODO abstract FreeRTOS specific calls so this can be removed
)

unset(THIS_LIB)

#********************************
# Gather I2S sources
#********************************
set(THIS_LIB I2S)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}" # TODO abstract FreeRTOS specific calls so this can be removed
)

unset(THIS_LIB)

#********************************
# Gather intertile sources
#********************************
set(THIS_LIB INTERTILE)
set(${THIS_LIB}_FLAGS "-O3")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}" # TODO abstract FreeRTOS specific calls so this can be removed
)

unset(THIS_LIB)

#********************************
# Gather mic array sources
#********************************
set(THIS_LIB MIC_ARRAY)
set(${THIS_LIB}_FLAGS "-Os")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}" # TODO abstract FreeRTOS specific calls so this can be removed
)

unset(THIS_LIB)

#********************************
# Gather rpc sources
#********************************
set(THIS_LIB RPC)
set(${THIS_LIB}_FLAGS "-O3")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/api"
)

unset(THIS_LIB)

#********************************
# Gather spi sources
#********************************
set(THIS_LIB SPI)
set(${THIS_LIB}_FLAGS "-O3")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/api"
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/${RTOS_CMAKE_RTOS}"
)

unset(THIS_LIB)

#********************************
# Gather trace sources
#********************************
set(THIS_LIB TRACE)
set(${THIS_LIB}_FLAGS "-O3")

file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})

set(${THIS_LIB}_INCLUDES
    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/api"
)

unset(THIS_LIB)

#**********************
# set user variables
#**********************
set(RTOS_SOURCES
    ${KERNEL_SOURCES}
    #${KERNEL_OPT_SOURCES}
    ${GPIO_SOURCES}
    ${I2C_SOURCES}
    ${I2S_SOURCES}
    ${INTERTILE_SOURCES}
    ${MIC_ARRAY_SOURCES}
    ${RPC_SOURCES}
    ${SPI_SOURCES}
    ${TRACE_SOURCES}
)

set(RTOS_INCLUDES
    ${KERNEL_INCLUDES}
    #${KERNEL_OPT_INCLUDES}
    ${GPIO_INCLUDES}
    ${I2C_INCLUDES}
    ${I2S_INCLUDES}
    ${INTERTILE_INCLUDES}
    ${MIC_ARRAY_INCLUDES}
    ${RPC_INCLUDES}
    ${SPI_INCLUDES}
    ${TRACE_INCLUDES}
)
