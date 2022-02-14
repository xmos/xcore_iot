#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES   src/*.c )
set(APP_INCLUDES src)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -O2
    -g
    -report
    -target=XCORE-AI-EXPLORER
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
)

set(APP_LINK_OPTIONS
    -report
    -target=XCORE-AI-EXPLORER
)

#**********************
# Tile Targets
#**********************
add_executable(test_hil_i2c_test_locks EXCLUDE_FROM_ALL)
target_sources(test_hil_i2c_test_locks PUBLIC ${APP_SOURCES})
target_include_directories(test_hil_i2c_test_locks PUBLIC ${APP_INCLUDES})
target_compile_definitions(test_hil_i2c_test_locks PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(test_hil_i2c_test_locks PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(test_hil_i2c_test_locks PUBLIC sdk::hil::lib_i2c sdk::utils)
target_link_options(test_hil_i2c_test_locks PRIVATE ${APP_LINK_OPTIONS})
set_target_properties(test_hil_i2c_test_locks PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
