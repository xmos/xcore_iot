query_tools_version()

if ((XTC_VERSION_MAJOR LESS 15) OR (XTC_VERSION_MINOR LESS 2))
    message(WARNING "XTC Tools version 15.2.0 or newer required for FreeRTOS xlink example")
    return()
endif()

#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/link
    ${CMAKE_CURRENT_LIST_DIR}/src/xlink_rx
    ${CMAKE_CURRENT_LIST_DIR}/src/xlink_tx
)
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
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XE_BASE_TILE=0

    XUD_CORE_CLOCK=600
)

set(RX_APP_COMPILER_FLAGS
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
)

set(TX_APP_COMPILER_FLAGS
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER_tx.xn
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
)

set(RX_APP_LINK_OPTIONS
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
)

set(TX_APP_LINK_OPTIONS
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER_tx.xn
)

set(APP_LINK_LIBRARIES
    example::freertos::xlink::bsp_config::xcore_ai_explorer_2V0
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_xlink_0)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} DEMO_TILE=0 THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${RX_APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} ${RX_APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_xlink_0)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} DEMO_TILE=0 THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${RX_APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} ${RX_APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_xlink_0 tile0_example_freertos_xlink_0 tile1_example_freertos_xlink_0 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_xlink_0)
create_debug_target(example_freertos_xlink_0)


#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_xlink_1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} DEMO_TILE=1 THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${TX_APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} ${TX_APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_xlink_1)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} DEMO_TILE=1 THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS} ${TX_APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} ${TX_APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_xlink_1 tile0_example_freertos_xlink_1 tile1_example_freertos_xlink_1 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_xlink_1)
create_debug_target(example_freertos_xlink_1)
create_install_target(example_freertos_xlink_0)
create_install_target(example_freertos_xlink_1)

#**********************
# Create custom target to build both example applications
#**********************
add_custom_target(example_freertos_xlink_both
    COMMAND
    DEPENDS
        example_freertos_xlink_0
        example_freertos_xlink_1
    COMMENT
        "Create both xlink example applications"
    VERBATIM
)