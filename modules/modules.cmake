cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(MODULES_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules")

set(LIB_XASSERT_DIR "${MODULES_DIR}/lib_xassert/lib_xassert")
set(LIB_DSP_DIR "${MODULES_DIR}/lib_dsp/lib_dsp")
set(LIB_LOGGING_DIR "${MODULES_DIR}/lib_logging/lib_logging")

#********************************
# Gather lib_xassert sources
#********************************
set(THIS_LIB LIB_XASSERT)
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
# Gather lib_dsp sources
#********************************
set(THIS_LIB LIB_DSP)
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
# Gather lib_logging sources
#********************************
set(THIS_LIB LIB_LOGGING)
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

#**********************
# set user variables
#**********************
set(MODULES_SOURCES
    ${LIB_XASSERT_SOURCES}
    ${LIB_DSP_SOURCES}
    ${LIB_LOGGING_SOURCES}
)

set(MODULES_INCLUDES
    ${LIB_XASSERT_INCLUDES}
    ${LIB_DSP_INCLUDES}
    ${LIB_LOGGING_INCLUDES}
)
