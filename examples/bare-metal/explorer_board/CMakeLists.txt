#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   src/*.c )
set(APP_INCLUDES
    src
    src/audio_pipeline
    src/platform
    src/misc
    src/demos
)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
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
    PLATFORM_USES_TILE_1=1

    MIC_ARRAY_BASIC_API_ENABLE=1
    MIC_ARRAY_CONFIG_MCLK_FREQ=24576000
    MIC_ARRAY_CONFIG_PDM_FREQ=3072000
    MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME=256
    MIC_ARRAY_CONFIG_MIC_COUNT=2
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(example_bare_metal_explorer_board EXCLUDE_FROM_ALL)
target_sources(example_bare_metal_explorer_board PUBLIC ${APP_SOURCES})
target_include_directories(example_bare_metal_explorer_board PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_bare_metal_explorer_board PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_bare_metal_explorer_board PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_bare_metal_explorer_board PUBLIC sdk::core sdk::hil_audio sdk::utils)
target_link_options(example_bare_metal_explorer_board PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(example_bare_metal_explorer_board)
create_debug_target(example_bare_metal_explorer_board)
