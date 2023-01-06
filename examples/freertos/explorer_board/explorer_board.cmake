#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

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
    XE_BASE_TILE=0
    XUD_CORE_CLOCK=600
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    rtos::bsp_config::xcore_ai_explorer
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_explorer_board)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_explorer_board)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_explorer_board tile0_example_freertos_explorer_board tile1_example_freertos_explorer_board 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_explorer_board)
create_debug_target(example_freertos_explorer_board)
create_install_target(example_freertos_explorer_board)

#**********************
# Filesystem support targets
#**********************
set(FATFS_CONTENTS_DIR ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/fatmktmp)
set(FATFS_FILE ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_explorer_board_fat.fs)
add_custom_target(
    example_freertos_explorer_board_fat.fs ALL
    COMMAND ${CMAKE_COMMAND} -E copy demo.txt ${FATFS_CONTENTS_DIR}/fs/demo.txt
    COMMAND fatfs_mkimage --input=${FATFS_CONTENTS_DIR} --output=${FATFS_FILE}
    COMMENT
        "Create filesystem"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
    VERBATIM
)

set_target_properties(example_freertos_explorer_board_fat.fs PROPERTIES
    ADDITIONAL_CLEAN_FILES "${FATFS_CONTENTS_DIR};${FATFS_FILE}"
)

create_filesystem_target(example_freertos_explorer_board)
create_flash_app_target(
    #[[ Target ]]                  example_freertos_explorer_board
    #[[ Boot Partition Size ]]     0x100000
    #[[ Data Partition Contents ]] ${FATFS_FILE}
    #[[ Dependencies ]]            make_fs_example_freertos_explorer_board
)
