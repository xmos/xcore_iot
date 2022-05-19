#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c )
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
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
    XUD_CORE_CLOCK=700

)

set(APP_LINK_OPTIONS
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
add_executable(example_bare_metal_uart EXCLUDE_FROM_ALL)
target_sources(example_bare_metal_uart PUBLIC ${APP_SOURCES})
target_include_directories(example_bare_metal_uart PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_bare_metal_uart PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_bare_metal_uart PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_bare_metal_uart PUBLIC sdk::core sdk::utils)
target_link_options(example_bare_metal_uart PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create run and debug targets
#**********************
create_run_target(example_bare_metal_uart)
create_debug_target(example_bare_metal_uart)
