#**********************
# Individual tests
#**********************
set(WIFI_TEST 1)

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
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    USB_TILE_NO=0
    USB_TILE=tile[USB_TILE_NO]
    XE_BASE_TILE=0
    XUD_CORE_CLOCK=600
    RUN_WIFI_TESTS=${WIFI_TEST}
    # SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR|SL_WFX_DEBUG_INIT\)
    SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR\)
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_test_rtos_driver_wifi)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC core::general rtos::freertos rtos::wifi)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_test_rtos_driver_wifi)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC core::general rtos::freertos rtos::wifi)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(test_rtos_driver_wifi tile0_test_rtos_driver_wifi tile1_test_rtos_driver_wifi 1)
