
## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-lquadspi"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/platform/aic3204.c"
    "src/platform/app_pll_ctrl.c"
    "src/platform/driver_instances.c"
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/example_pipeline.c"
    "src/gpio_ctrl/gpio_ctrl.c"
    "src/mem_analysis/mem_analysis.c"
    "src/filesystem/filesystem_demo.c"
)

set(APP_INCLUDES
    "src"
)

set(APP_COMPILE_DEFINITIONS
    "DEBUG_PRINT_ENABLE=1"
    "XCOREAI_EXPLORER=1"
    "PLATFORM_SUPPORTS_TILE_0=1"
    "PLATFORM_SUPPORTS_TILE_1=1"
    "PLATFORM_SUPPORTS_TILE_2=0"
    "PLATFORM_SUPPORTS_TILE_3=0"
    "USB_TILE_NO=0"
    "USB_TILE=tile[USB_TILE_NO]"
    "XE_BASE_TILE=0"
)

set(APP_LINK_OPTIONS
    "-lquadspi"
    "-report"
    "${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
)

#**********************
# targets
#**********************
set(TARGET_NAME explorer_board_tile0)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PRIVATE ${APP_COMPILE_DEFINITIONS} PLATFORM_USES_TILE_0=1 PLATFORM_USES_TILE_1=1 THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PRIVATE xcore_sdk)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME explorer_board_tile1)
add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PRIVATE ${APP_COMPILE_DEFINITIONS} PLATFORM_USES_TILE_0=1 PLATFORM_USES_TILE_1=1 THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PRIVATE xcore_sdk)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

## create_multitile_target(TARGET_NAME explorer_board_tile0 explorer_board_tile1)

## Hard code a target for now
add_custom_target(explorer_board
    ALL
    COMMAND xobjdump --split --split-dir ${CMAKE_BINARY_DIR}/examples/freertos/explorer_board explorer_board_tile1
    COMMAND xobjdump ${CMAKE_BINARY_DIR}/examples/freertos/explorer_board/explorer_board_tile0 -r 0,1,${CMAKE_BINARY_DIR}/examples/freertos/explorer_board/image_n0c1_2.elf
    COMMAND cp ${CMAKE_BINARY_DIR}/examples/freertos/explorer_board/explorer_board_tile0 explorer_board.xe
    DEPENDS
        explorer_board_tile0
        explorer_board_tile1
    BYPRODUCTS
        #TODO
    COMMENT "Merging tile ${_TILE} into ${XE_BASE_TILE}.xe"
    VERBATIM
)
