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
    -target=XCORE-AI-EXPLORER
)
set(APP_LINK_OPTIONS
    -report
    -target=XCORE-AI-EXPLORER
)

#**********************
# Tile Targets
#**********************
if(NOT DEFINED ENV{NUMS_IN_OUT})
    set(NUMS_IN_OUT "4;4" "1;1" "0;4" "4;0" "2;2")
else()
    set(NUMS_IN_OUT $ENV{NUMS_IN_OUT})
endif()
if(NOT DEFINED ENV{TEST_LEVEL})
    set(STOPS stop no_stop)
else()
    set(STOPS $ENV{STOPS})
endif()

set(TEST_LEVELS 0 1)  ## 1 == smoke, 0 == nightly

#**********************
# Setup targets
#**********************
list(LENGTH NUMS_IN_OUT NUMS_IN_OUT_LEN)
math(EXPR num_pairs "${NUMS_IN_OUT_LEN} / 2")

foreach(i RANGE 1 ${num_pairs})
    list(POP_FRONT NUMS_IN_OUT num_in num_out)
    foreach(test_level ${TEST_LEVELS})
        set(TARGET_NAME "test_hil_i2s_slave_test_${test_level}_${num_in}_${num_out}")
        add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
        target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
        target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                ${APP_COMPILE_DEFINITIONS}
                NUM_OUT=${num_out}
                NUM_IN=${num_in}
                SMOKE=${test_level}
                SLAVE_INVERT_BCLK=0
        )
        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_i2s)
        target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
        set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
        unset(TARGET_NAME)

        set(TARGET_NAME "test_hil_i2s_slave_test_${test_level}_${num_in}_${num_out}_inv")
        add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
        target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
        target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                ${APP_COMPILE_DEFINITIONS}
                NUM_OUT=${num_out}
                NUM_IN=${num_in}
                SMOKE=${test_level}
                SLAVE_INVERT_BCLK=1
        )
        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_i2s)
        target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
        set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
        unset(TARGET_NAME)
    endforeach()
endforeach()
