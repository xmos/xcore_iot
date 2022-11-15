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
set(TARGET_NAME tile0_example_freertos_dfu_v1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 VERSION=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_dfu_v1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 VERSION=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile0_example_freertos_dfu_v2)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 VERSION=2)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_dfu_v2)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 VERSION=2)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_dfu_v1 tile0_example_freertos_dfu_v1 tile1_example_freertos_dfu_v1 1)
merge_binaries(example_freertos_dfu_v2 tile0_example_freertos_dfu_v2 tile1_example_freertos_dfu_v2 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_dfu_v1)
create_debug_target(example_freertos_dfu_v1)
create_flash_app_target(
    #[[ Target ]]                 example_freertos_dfu_v1
    #[[ Boot Partition Size ]]    0x100000 
    #[[ Data Parition Contents ]] 
    #[[ Dependencies ]]           
)
query_tools_version()
create_upgrade_img_target(example_freertos_dfu_v1 ${XTC_VERSION_MAJOR} ${XTC_VERSION_MINOR})
create_install_target(example_freertos_dfu_v1)
create_erase_all_target(example_freertos_dfu_v1 ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn)

create_run_target(example_freertos_dfu_v2)
create_debug_target(example_freertos_dfu_v2)
create_flash_app_target(
    #[[ Target ]]                 example_freertos_dfu_v2
    #[[ Boot Partition Size ]]    0x100000 
    #[[ Data Parition Contents ]] 
    #[[ Dependencies ]]           
)
create_upgrade_img_target(example_freertos_dfu_v2 ${XTC_VERSION_MAJOR} ${XTC_VERSION_MINOR})
create_install_target(example_freertos_dfu_v2)
create_erase_all_target(example_freertos_dfu_v2 ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn)
