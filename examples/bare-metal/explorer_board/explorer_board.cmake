#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/audio_pipeline
    ${CMAKE_CURRENT_LIST_DIR}/src/platform
    ${CMAKE_CURRENT_LIST_DIR}/src/misc
    ${CMAKE_CURRENT_LIST_DIR}/src/demos
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
    PLATFORM_USES_TILE_1=1

    MIC_ARRAY_CONFIG_CLOCK_BLOCK_A=XS1_CLKBLK_1
    MIC_ARRAY_CONFIG_CLOCK_BLOCK_B=XS1_CLKBLK_2
    MIC_ARRAY_CONFIG_PORT_MCLK=PORT_MCLK_IN
    MIC_ARRAY_CONFIG_PORT_PDM_CLK=PORT_PDM_CLK
    MIC_ARRAY_CONFIG_PORT_PDM_DATA=PORT_PDM_DATA
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
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

# MCLK_FREQ,  PDM_FREQ, MIC_COUNT,  SAMPLES_PER_FRAME
mic_array_vanilla_add( example_bare_metal_explorer_board
    24576000  3072000   2           240 )

#**********************
# Create run and debug targets
#**********************
create_run_target(example_bare_metal_explorer_board)
create_debug_target(example_bare_metal_explorer_board)
create_flash_app_target(example_bare_metal_explorer_board)
