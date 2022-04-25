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
set(APP_COMPILE_DEFINITIONS
    SIM=1
    GENERATE_MCLK=1
)
set(APP_LINK_OPTIONS
    -report
    -target=XCORE-AI-EXPLORER
)

#**********************
# Tile Targets
#**********************
if(NOT DEFINED ENV{SAMPLE_RATES})
    set(SAMPLE_RATES 768000 384000 192000)
else()
    set(SAMPLE_RATES $ENV{SAMPLE_RATES})
endif()
if(NOT DEFINED ENV{CHANS})
    set(CHANS 1 2 3 4)
else()
    set(CHANS $ENV{CHANS})
endif()
if(NOT DEFINED ENV{RX_TX_INCS})
    set(RX_TX_INCS "5;5" "10;0" "0;10")
else()
    set(RX_TX_INCS $ENV{RX_TX_INCS})
endif()

#**********************
# Setup targets
#**********************
list(LENGTH RX_TX_INCS RX_TX_INCS_LEN)
math(EXPR num_inc_pairs "${RX_TX_INCS_LEN} / 2")

foreach(i RANGE 1 ${num_inc_pairs})
    list(POP_FRONT RX_TX_INCS rx_inc tx_inc)
    foreach(rate ${SAMPLE_RATES})
        foreach(chan ${CHANS})
            set(TARGET_NAME "test_hil_backpressure_test_${rate}_${chan}_${rx_inc}_${tx_inc}")
            add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
            target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
            target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
            target_compile_definitions(${TARGET_NAME}
                PRIVATE
                    ${APP_COMPILE_DEFINITIONS}
                    SAMPLE_FREQUENCY=${rate}
                    NUM_I2S_LINES=${chan}
                    RECEIVE_DELAY_INCREMENT=${rx_inc}
                    SEND_DELAY_INCREMENT=${tx_inc}
            )
            target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
            target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_i2s)
            target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
            set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
            unset(TARGET_NAME)
        endforeach()
    endforeach()
endforeach()
