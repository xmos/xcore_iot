
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Source files
    file(GLOB_RECURSE LIB_C_SOURCES   lib_mic_array/lib_mic_array/src/*.c )
    file(GLOB_RECURSE LIB_CXX_SOURCES lib_mic_array/lib_mic_array/src/*.cc)
    file(GLOB_RECURSE LIB_XC_SOURCES  lib_mic_array/lib_mic_array/src/*.xc)
    file(GLOB_RECURSE LIB_ASM_SOURCES lib_mic_array/lib_mic_array/src/*.S )

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})

    ## Set any local library compile options
    set(LIB_COMPILE_FLAGS "-O3")

    ## Includes files
    set(LIB_PUBLIC_INCLUDES
        lib_mic_array/lib_mic_array/api
        lib_mic_array/lib_mic_array/src/fir
    )
    set(LIB_PRIVATE_INCLUDES lib_mic_array/lib_mic_array/src)

    ## Gather library sources
    set(LIB_PUBLIC_SOURCES   "")
    set(LIB_PRIVATE_SOURCES  ${LIB_C_SOURCES} ${LIB_CXX_SOURCES} ${LIB_XC_SOURCES})

    ## Append platform specific sources
    list(APPEND LIB_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

    ## Create library target
    add_library(xcore_sdk_hil_lib_mic_array INTERFACE)
    target_sources(xcore_sdk_hil_lib_mic_array
        INTERFACE
            ${LIB_PUBLIC_SOURCES}
            ${LIB_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_hil_lib_mic_array
        INTERFACE
            ${LIB_PUBLIC_INCLUDES}
            ${LIB_PRIVATE_INCLUDES}
    )
    target_compile_options(xcore_sdk_hil_lib_mic_array
        INTERFACE
            ${LIB_COMPILE_FLAGS}
    )
    target_link_libraries(xcore_sdk_hil_lib_mic_array
        INTERFACE
            sdk::lib_xs3_math
    )

    ## Create an alias
    add_library(sdk::hil::lib_mic_array ALIAS xcore_sdk_hil_lib_mic_array)
endif()
