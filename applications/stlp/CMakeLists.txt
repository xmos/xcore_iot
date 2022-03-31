

add_subdirectory(bsp_config)

set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
)

set(APP_SOURCES
    "src/main.c"

    "src/app_control/app_control.c"
    "src/app_control/ap_control_handlers.c"
    "src/app_control/aec_control_handlers.c"
    "src/app_control/stage1_control_handlers.c"
    "src/app_control/stage2_control_handlers.c"

    "src/audio_pipeline/audio_pipeline_t0.c"
    "src/audio_pipeline/audio_pipeline_t1.c"
    "src/audio_pipeline/aec/aec_process_frame_1thread.c"

    "src/usb/usb_audio.c"
    "src/usb/usb_descriptors.c"
    "src/usb/adaptive_rate_adjust.c"

    "src/ww_model_runner/ww_model_runner.c"
    "src/ww_model_runner/model_runner.c"

    "src/gpio_test/gpio_test.c"
)

set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/usb/adaptive_rate_adjust.c PROPERTIES COMPILE_FLAGS "-O3")
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/src/vfe_pipeline/burn_cycles.S PROPERTIES LANGUAGE ASM)

set(APP_INCLUDES
    "src"
    "src/vfe_pipeline"
    "src/usb"
    "src/ww_model_runner"
)

set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XUD_CORE_CLOCK=600

    CFG_TUSB_DEBUG_PRINTF=rtos_printf
    CFG_TUSB_DEBUG=0
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    sdk::core
    sdk::rtos_freertos
    sdk::rtos::usb_device_control
    sdk::rtos::audio_drivers
    sdk::lib_src
    avona::adec
    avona::aec
    avona::agc
    avona::ic
    avona::ns
    avona::vad

    sdk::app::stlp::xk_voice_l71

    # sdk::app::stlp::xcore_ai_explorer
)
#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_application_stlp)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_application_stlp)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS} )
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(application_stlp tile0_application_stlp tile1_application_stlp 1)

#**********************
# Create run and debug targets
#**********************
create_run_target(application_stlp)
create_debug_target(application_stlp)
