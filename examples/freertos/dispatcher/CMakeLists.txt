#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   src/*.c )
set(APP_INCLUDES src)

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
    PLATFORM_USES_TILE_1=0
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(dispatcher EXCLUDE_FROM_ALL)
target_sources(dispatcher PUBLIC ${APP_SOURCES})
target_include_directories(dispatcher PUBLIC ${APP_INCLUDES})
target_compile_definitions(dispatcher PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(dispatcher PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(dispatcher PUBLIC sdk::core sdk::rtos_freertos)
target_link_options(dispatcher PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(dispatcher)
create_debug_target(dispatcher)
