
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Source files
    file(GLOB_RECURSE LIB_C_SOURCES   lib_l2_cache/lib_l2_cache/src/*.c )
    file(GLOB_RECURSE LIB_CXX_SOURCES lib_l2_cache/lib_l2_cache/src/*.cc)
    file(GLOB_RECURSE LIB_XC_SOURCES  lib_l2_cache/lib_l2_cache/src/*.xc)
    file(GLOB_RECURSE LIB_ASM_SOURCES lib_l2_cache/lib_l2_cache/src/*.S )

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})

    ## Set any local library compile options
    set(LIB_COMPILE_FLAGS "-Os")

    ## Includes files
    set(LIB_PUBLIC_INCLUDES  lib_l2_cache/lib_l2_cache/api)
    set(LIB_PRIVATE_INCLUDES lib_l2_cache/lib_l2_cache/src)

    ## Gather library sources
    set(LIB_PUBLIC_SOURCES   "")
    set(LIB_PRIVATE_SOURCES  ${LIB_C_SOURCES} ${LIB_CXX_SOURCES} ${LIB_XC_SOURCES})

    ## Append platform specific sources
    list(APPEND LIB_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

    ## Create library target
    add_library(xcore_sdk_hil_lib_l2_cache STATIC)
    target_sources(xcore_sdk_hil_lib_l2_cache
        PUBLIC
            ${LIB_PUBLIC_SOURCES}
        PRIVATE
            ${LIB_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_hil_lib_l2_cache
        PUBLIC
            ${LIB_PUBLIC_INCLUDES}
        PRIVATE
            ${LIB_PRIVATE_INCLUDES}
    )
    target_compile_options(xcore_sdk_hil_lib_l2_cache
        PRIVATE
            ${LIB_COMPILE_FLAGS}
    )
    target_link_libraries(xcore_sdk_hil_lib_l2_cache
        PUBLIC
            sdk::utils
    )

    ## Create an alias
    add_library(sdk::hil::lib_l2_cache ALIAS xcore_sdk_hil_lib_l2_cache)
endif()
