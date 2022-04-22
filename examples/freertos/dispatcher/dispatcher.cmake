#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

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
    PLATFORM_USES_TILE_1=0
)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(example_freertos_dispatcher EXCLUDE_FROM_ALL)
target_sources(example_freertos_dispatcher PUBLIC ${APP_SOURCES})
target_include_directories(example_freertos_dispatcher PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_freertos_dispatcher PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_freertos_dispatcher PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_freertos_dispatcher PUBLIC sdk::core sdk::rtos_freertos)
target_link_options(example_freertos_dispatcher PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_dispatcher)
create_debug_target(example_freertos_dispatcher)
