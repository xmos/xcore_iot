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
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(explorer_board_bare_metal EXCLUDE_FROM_ALL)
target_sources(explorer_board_bare_metal PUBLIC ${APP_SOURCES})
target_include_directories(explorer_board_bare_metal PUBLIC ${APP_INCLUDES})
target_compile_definitions(explorer_board_bare_metal PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(explorer_board_bare_metal PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(explorer_board_bare_metal PUBLIC sdk::core)
target_link_options(explorer_board_bare_metal PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(explorer_board_bare_metal)
create_debug_target(explorer_board_bare_metal)
