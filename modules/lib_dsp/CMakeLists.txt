
## Source files
file(GLOB_RECURSE LIB_C_SOURCES   lib_dsp/lib_dsp/src/*.c )
file(GLOB_RECURSE LIB_CXX_SOURCES lib_dsp/lib_dsp/src/*.cc)
file(GLOB_RECURSE LIB_XC_SOURCES  lib_dsp/lib_dsp/src/*.xc)
file(GLOB_RECURSE LIB_ASM_SOURCES lib_dsp/lib_dsp/src/*.S )

## cmake doesn't recognize .S files as assembly by default
set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

## Assume all asm is XS3A for now
set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})

## Set any local library compile options
set(LIB_COMPILE_FLAGS "")

## Includes files
set(LIB_PUBLIC_INCLUDES  lib_dsp/lib_dsp/api)
set(LIB_PRIVATE_INCLUDES lib_dsp/lib_dsp/src)

## Gather library sources
set(LIB_PUBLIC_SOURCES   "")
set(LIB_PRIVATE_SOURCES  ${LIB_C_SOURCES} ${LIB_CXX_SOURCES} ${LIB_XC_SOURCES})

## Append platform specific sources
list(APPEND LIB_PRIVATE_SOURCES ${${CMAKE_SYSTEM_NAME}_SOURCES})

## Create library target
add_library(xcore_sdk_lib_dsp STATIC)
target_sources(xcore_sdk_lib_dsp
    PUBLIC
        ${LIB_PUBLIC_SOURCES}
    PRIVATE
        ${LIB_PRIVATE_SOURCES}
)
target_include_directories(xcore_sdk_lib_dsp
    PUBLIC
        ${LIB_PUBLIC_INCLUDES}
    PRIVATE
        ${LIB_PRIVATE_INCLUDES}
)
target_compile_options(xcore_sdk_lib_dsp
    PRIVATE
        ${LIB_COMPILE_FLAGS}
)
target_link_libraries(xcore_sdk_lib_dsp
    PUBLIC
        sdk::legacy_compat
)

## Create an alias
add_library(sdk::lib_dsp ALIAS xcore_sdk_lib_dsp)
