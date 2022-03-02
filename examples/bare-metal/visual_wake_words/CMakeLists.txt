#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   app/*.c app/*.xc model_runner/*.c model_runner/*.cc)
set(APP_INCLUDES app model_runner)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/config.xscope
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    TF_LITE_STATIC_MEMORY=1
    XCORE=1
    # NDEBUG                        # define this to remove debug and profiling
    # TF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging
)

set(APP_LINK_OPTIONS
    -lquadspi
    -w
    -report
    ${CMAKE_CURRENT_SOURCE_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_SOURCE_DIR}/config.xscope
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME example_bare_metal_vww)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS})
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC sdk::aif_model_runner_bare_metal)
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Create run targets
#**********************
add_custom_target(run_example_bare_metal_vww
  COMMAND xrun --xscope-port localhost:10234 example_bare_metal_vww.xe
  DEPENDS example_bare_metal_vww
  COMMENT
    "Run application"
  VERBATIM
)

add_custom_target(xsim_example_bare_metal_vww
  COMMAND xsim --xscope "-realtime localhost:10234" example_bare_metal_vww.xe
  DEPENDS example_bare_metal_vww
  COMMENT
    "Run application"
  VERBATIM
)
