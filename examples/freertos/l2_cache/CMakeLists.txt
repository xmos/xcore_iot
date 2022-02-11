#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   src/*.c )
set(APP_INCLUDES src)

set_source_files_properties(src/example_code.c PROPERTIES COMPILE_FLAGS -O0)
#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -O2
    -g
    -report
    -fxscope
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=0
    USE_SWMEM=1
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(l2_cache EXCLUDE_FROM_ALL)
target_sources(l2_cache PUBLIC ${APP_SOURCES})
target_include_directories(l2_cache PUBLIC ${APP_INCLUDES})
target_compile_definitions(l2_cache PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(l2_cache PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(l2_cache PUBLIC sdk::core sdk::rtos_freertos)
target_link_options(l2_cache PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(l2_cache)
create_debug_target(l2_cache)

#**********************
# Extract swmem
#**********************
add_custom_command(
    TARGET l2_cache POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory l2_cache_split
    COMMAND xobjdump --strip l2_cache.xe
    COMMAND xobjdump --split --split-dir l2_cache_split l2_cache.xb
    BYPRODUCTS
        l2_cache.xb
        l2_cache_split
    VERBATIM
)

#**********************
# SWMEM flashing support targets
#**********************
add_custom_target(flash_l2_cache_swmem
    COMMAND xflash --write-all ${CMAKE_CURRENT_BINARY_DIR}/l2_cache_split/image_n0c0.swmem --target-file ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    DEPENDS l2_cache
    COMMENT
        "Flash swmem"
    VERBATIM
)
