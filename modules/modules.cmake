cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(MODULES_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules")

set(LIB_DSP_DIR "${MODULES_DIR}/lib_dsp")
set(LIB_LOGGING_DIR "${MODULES_DIR}/lib_logging")
set(LIB_RANDOM_DIR "${MODULES_DIR}/lib_random")
set(LIB_XS3_MATH_DIR "${MODULES_DIR}/lib_xs3_math")
set(LEGACY_COMPAT_DIR "${MODULES_DIR}/legacy_compat")

#**********************
# Options
#**********************
option(USE_LIB_DSP "Enable to include lib_dsp" TRUE)
option(USE_LIB_LOGGING "Enable to include lib_logging" TRUE)
option(USE_LIB_RANDOM "Enable to include lib_random" TRUE)
option(USE_LIB_XS3_MATH "Enable to include lib_xs3_math" FALSE)  # Currently not used
option(USE_LEGACY_COMPAT "Enable to include legacy compatibility layer for XMOS libraries" TRUE)

#********************************
# Gather lib_dsp sources
#********************************
set(THIS_LIB LIB_DSP)
if(${USE_${THIS_LIB}})
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
endif()
unset(THIS_LIB)

#********************************
# Gather lib_logging sources
#********************************
set(THIS_LIB LIB_LOGGING)
if(${USE_${THIS_LIB}})
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
endif()
unset(THIS_LIB)

#********************************
# Gather lib_random sources
#********************************
set(THIS_LIB LIB_RANDOM)
if(${USE_${THIS_LIB}})
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
endif()
unset(THIS_LIB)

#********************************
# Gather lib_xs3_math sources
#********************************
set(THIS_LIB LIB_XS3_MATH)
if(${USE_${THIS_LIB}})
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
endif()
unset(THIS_LIB)

#********************************
# Gather legacy compat sources
#********************************
set(THIS_LIB LEGACY_COMPAT)
if(${USE_${THIS_LIB}})
    set(${THIS_LIB}_SOURCES "")
    set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}"
    )
endif()
unset(THIS_LIB)

#**********************
# set user variables
#**********************
set(MODULES_SOURCES
    ${LIB_DSP_SOURCES}
    ${LIB_LOGGING_SOURCES}
    ${LIB_RANDOM_SOURCES}
    ${LIB_XS3_MATH_SOURCES}
    ${LEGACY_COMPAT_SOURCES}
)

set(MODULES_INCLUDES
    ${LIB_DSP_INCLUDES}
    ${LIB_LOGGING_INCLUDES}
    ${LIB_RANDOM_INCLUDES}
    ${LIB_XS3_MATH_INCLUDES}
    ${LEGACY_COMPAT_INCLUDES}
)

list(REMOVE_DUPLICATES MODULES_SOURCES)
list(REMOVE_DUPLICATES MODULES_INCLUDES)
