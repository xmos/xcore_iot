#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/example_code.c PROPERTIES COMPILE_FLAGS -O0)

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
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
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
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(example_freertos_l2_cache EXCLUDE_FROM_ALL)
target_sources(example_freertos_l2_cache PUBLIC ${APP_SOURCES})
target_include_directories(example_freertos_l2_cache PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_freertos_l2_cache PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_freertos_l2_cache PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_freertos_l2_cache PUBLIC sdk::core sdk::rtos_freertos)
target_link_options(example_freertos_l2_cache PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_l2_cache)
create_debug_target(example_freertos_l2_cache)

#**********************
# Extract swmem
#**********************
add_custom_command(
    TARGET example_freertos_l2_cache POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory example_freertos_l2_cache_split
    COMMAND xobjdump --strip example_freertos_l2_cache.xe
    COMMAND xobjdump --split --split-dir example_freertos_l2_cache_split example_freertos_l2_cache.xb
    BYPRODUCTS
        example_freertos_l2_cache.xb
        example_freertos_l2_cache_split
    VERBATIM
)

#**********************
# SWMEM flashing support targets
#**********************
add_custom_target(flash_example_freertos_l2_cache_swmem
    COMMAND xflash --write-all ${CMAKE_CURRENT_BINARY_DIR}/example_freertos_l2_cache_split/image_n0c0.swmem --target-file ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    DEPENDS example_freertos_l2_cache
    COMMENT
        "Flash swmem"
    VERBATIM
)
