
if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Source files
    file(GLOB_RECURSE LIB_C_SOURCES   src/*.c )
    file(GLOB_RECURSE LIB_CXX_SOURCES src/*.cc)
    file(GLOB_RECURSE LIB_XC_SOURCES  src/*.xc)
    file(GLOB_RECURSE LIB_ASM_SOURCES src/*.S )

    ## cmake doesn't recognize .S files as assembly by default
    set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

    ## Assume all asm is XS3A for now
    set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})

    ## Set any local library compile options
    set(LIB_COMPILE_FLAGS "")

    ## Includes files
    set(LIB_PUBLIC_INCLUDES  api src)
    set(LIB_PRIVATE_INCLUDES "")

    ## Gather library sources
    set(LIB_PUBLIC_SOURCES   ${LIB_C_SOURCES} ${LIB_CXX_SOURCES} ${LIB_XC_SOURCES})
    set(LIB_PRIVATE_SOURCES  "")

    ## Append platform specific sources
    list(APPEND LIB_PUBLIC_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

    ## Create library target
    ## INTERFACE because we depend on the app specific debug flags
    add_library(xcore_sdk_utils INTERFACE)
    target_sources(xcore_sdk_utils
        INTERFACE
            ${LIB_PUBLIC_SOURCES}
            ${LIB_PRIVATE_SOURCES}
    )
    target_include_directories(xcore_sdk_utils
        INTERFACE
            ${LIB_PUBLIC_INCLUDES}
            ${LIB_PRIVATE_INCLUDES}
    )
    target_compile_options(xcore_sdk_utils
        INTERFACE
            $<$<CONFIG:Debug>:-DDEBUG_PRINT_ENABLE=1>
    )

    ## Create an alias
    add_library(sdk::utils ALIAS xcore_sdk_utils)
endif()
