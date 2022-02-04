
## Target name
set( LIB_NAME   utils )

## Source files
file( GLOB_RECURSE ${LIB_NAME}_C_SOURCES   src/*.c  )
file( GLOB_RECURSE ${LIB_NAME}_CXX_SOURCES src/*.cc )
file( GLOB_RECURSE ${LIB_NAME}_XC_SOURCES  src/*.xc )
file( GLOB_RECURSE ${LIB_NAME}_ASM_SOURCES src/*.S  )

## cmake doesn't recognize .S files as assembly by default
set_source_files_properties( ${LIB_NAME}_ASM_SOURCES PROPERTIES LANGUAGE ASM )

## Includes files
set( ${LIB_NAME}_INCLUDES  api )

## Gather library sources
set( ${LIB_NAME}_SOURCES_XCORE ${${LIB_NAME}_ASM_SOURCES} )
set( ${LIB_NAME}_SOURCES ${${LIB_NAME}_C_SOURCES} ${${LIB_NAME}_CXX_SOURCES} ${${LIB_NAME}_XC_SOURCES} )

## Append platform specific sources
list( APPEND ${LIB_NAME}_SOURCES   ${${LIB_NAME}_SOURCES_${CMAKE_SYSTEM_NAME}} )

## Set compile flags
set( ${${LIB_NAME}_COMPILE_FLAGS} -march=xs3a)

## Create library target
add_library( ${LIB_NAME} STATIC )
target_sources( ${LIB_NAME} PUBLIC ${${LIB_NAME}_SOURCES} )
target_include_directories( ${LIB_NAME} PUBLIC ${${LIB_NAME}_INCLUDES} )
target_compile_options( ${LIB_NAME} PRIVATE ${${LIB_NAME}_COMPILE_FLAGS})
