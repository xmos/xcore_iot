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
set(VERSIONS 1 2 3)

foreach(VERSION ${VERSIONS})
    set(TARGET_NAME tile0_example_freertos_dfu_v${VERSION})
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
    target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
    target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0 VERSION=${VERSION})
    target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
    target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
    unset(TARGET_NAME)

    set(TARGET_NAME tile1_example_freertos_dfu_v${VERSION})
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
    target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
    target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1 VERSION=${VERSION})
    target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
    target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
    unset(TARGET_NAME)

    #**********************
    # Merge binaries
    #**********************
    merge_binaries(example_freertos_dfu_v${VERSION} tile0_example_freertos_dfu_v${VERSION} tile1_example_freertos_dfu_v${VERSION} 1)

    #**********************
    # Create run and debug targets
    #**********************
    create_run_target(example_freertos_dfu_v${VERSION})
    create_debug_target(example_freertos_dfu_v${VERSION})
    create_flash_app_target(
        #[[ Target ]]                 example_freertos_dfu_v${VERSION}
        #[[ Boot Partition Size ]]    0x100000 
        #[[ Data Parition Contents ]] 
        #[[ Dependencies ]]           
    )
    query_tools_version()
    create_upgrade_img_target(example_freertos_dfu_v${VERSION} ${XTC_VERSION_MAJOR} ${XTC_VERSION_MINOR})
    create_install_target(example_freertos_dfu_v${VERSION})
    create_erase_all_target(example_freertos_dfu_v${VERSION} ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn)

    #**********************
    # Create loader
    #**********************
    create_loader_target(example_freertos_dfu_v${VERSION} ${CMAKE_CURRENT_LIST_DIR}/loader/loader.c)
endforeach()

add_custom_target(example_freertos_dfu_loader_flash
    COMMAND xflash --quad-spi-clock 50MHz --factory example_freertos_dfu_v1.xe --upgrade 1 example_freertos_dfu_v2 --loader example_freertos_dfu_v1_loader.o
    DEPENDS
        example_freertos_dfu_v1_loader
        example_freertos_dfu_v1
        example_freertos_dfu_v2
)
