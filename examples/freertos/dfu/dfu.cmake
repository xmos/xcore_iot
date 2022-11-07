#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

#**********************
# Import example specific bsp_config
#**********************
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/bsp_config)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -lquadspi
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    
    XUD_CORE_CLOCK=600
)

set(APP_LINK_OPTIONS
    -lquadspi
    -fxscope
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    example::freertos::dfu::bsp_config::xcore_ai_explorer
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_dfu)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} 
"-Wm,--map,memory0.map")
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_dfu)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS}
"-Wm,--map,memory1.map")
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_dfu tile0_example_freertos_dfu tile1_example_freertos_dfu 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_dfu)
create_debug_target(example_freertos_dfu)
create_flash_app_target(example_freertos_dfu)
