
## Source files
file(GLOB_RECURSE LIB_C_SOURCES
    lib_xs3_math/lib_xs3_math/src/bfp/*.c
    lib_xs3_math/lib_xs3_math/src/vect/*.c
    lib_xs3_math/lib_xs3_math/src/scalar/*.c
)
file(GLOB_RECURSE LIB_CXX_SOURCES lib_xs3_math/lib_xs3_math/src/*.cc)
file(GLOB_RECURSE LIB_XC_SOURCES  lib_xs3_math/lib_xs3_math/src/*.xc)
file(GLOB_RECURSE LIB_ASM_SOURCES lib_xs3_math/lib_xs3_math/src/*.S )
list(APPEND LIB_C_SOURCES    lib_xs3_math/lib_xs3_math/src/etc/xs3_fft_lut/xs3_fft_lut.c)

## cmake doesn't recognize .S files as assembly by default
set_source_files_properties(LIB_ASM_SOURCES PROPERTIES LANGUAGE ASM)

## Assume all asm is XS3A for now
set(XCORE_XS3A_SOURCES ${LIB_ASM_SOURCES})
list(APPEND XCORE_XS3A_SOURCES ${LIB_XC_SOURCES})

## Set any local library compile options
set(LIB_COMPILE_FLAGS "-Os")

## Includes files
set(LIB_PUBLIC_INCLUDES
    lib_xs3_math/lib_xs3_math/api
    lib_xs3_math/lib_xs3_math/api/bfp
    lib_xs3_math/lib_xs3_math/api/scalar
    lib_xs3_math/lib_xs3_math/api/vect
    lib_xs3_math/lib_xs3_math/src/etc/xs3_fft_lut
)
set(LIB_PRIVATE_INCLUDES lib_xs3_math/lib_xs3_math/src)

## Gather library sources
set(LIB_PUBLIC_SOURCES   "")
set(LIB_PRIVATE_SOURCES  ${LIB_C_SOURCES} ${LIB_CXX_SOURCES})

## Append platform specific sources
if(CMAKE_SYSTEM_NAME STREQUAL XCORE_XS3A)
    list(APPEND LIB_PRIVATE_SOURCES ${XCORE_XS3A_SOURCES})
    list(APPEND LIB_PUBLIC_INCLUDES lib_xs3_math/lib_xs3_math/src/arch/xcore)
else()
    file(GLOB_RECURSE X86_SOURCES lib_xs3_math/lib_xs3_math/src/arch/ref/*.c)
    list(APPEND LIB_PRIVATE_SOURCES ${X86_SOURCES})
    list(APPEND LIB_PUBLIC_INCLUDES lib_xs3_math/lib_xs3_math/src/arch/ref)
endif()

## Create library target
add_library(xcore_sdk_lib_xs3_math STATIC)
target_sources(xcore_sdk_lib_xs3_math
    PUBLIC
        ${LIB_PUBLIC_SOURCES}
    PRIVATE
        ${LIB_PRIVATE_SOURCES}
)
target_include_directories(xcore_sdk_lib_xs3_math
    PUBLIC
        ${LIB_PUBLIC_INCLUDES}
    PRIVATE
        ${LIB_PRIVATE_INCLUDES}
)
target_compile_options(xcore_sdk_lib_xs3_math
    PRIVATE
        ${LIB_COMPILE_FLAGS}
)
target_link_libraries(xcore_sdk_lib_xs3_math
    PUBLIC
        $<$<PLATFORM_ID:XCORE_XS3A>:sdk::legacy_compat>
)

## Create an alias
add_library(sdk::lib_xs3_math ALIAS xcore_sdk_lib_xs3_math)
