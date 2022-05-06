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
if(NOT DEFINED ENV{TEST_BAUD})
    set(TEST_BAUD 1843200 921600 576000 115200 38400 19200 9600 1200)
else()
    set(TEST_BAUD $ENV{TEST_BAUD})
endif()
if(NOT DEFINED ENV{TEST_DATA_BITS})
    set(TEST_DATA_BITS 8 7 6 5)
else()
    set(TEST_DATA_BITS $ENV{TEST_DATA_BITS})
endif()
if(NOT DEFINED ENV{TEST_PARITY})
    set(TEST_PARITY NONE EVEN ODD)
else()
    set(STOPS $ENV{TEST_PARITY})
endif()

if(NOT DEFINED ENV{TEST_STOP_BITS})
    set(TEST_STOP_BITS 1 2)
else()
    set(STOPS $ENV{TEST_STOP_BITS})
endif()


#**********************
# Setup targets
#**********************
foreach(baud ${TEST_BAUD})
    foreach(data ${TEST_DATA_BITS})
        foreach(parity ${TEST_PARITY})
            foreach(stop ${TEST_STOP_BITS})
                set(TARGET_NAME "test_hil_uart_tx_test_${baud}_${data}_${parity}_${stop}")
                message(STATUS "${TARGET_NAME}") #Print so we can copy to build_tests sh file
                add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
                target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
                target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
                target_compile_definitions(${TARGET_NAME}
                    PRIVATE
                        ${APP_COMPILE_DEFINITIONS}
                        TEST_BAUD=${baud}
                        TEST_DATA_BITS=${data}
                        TEST_PARITY=UART_PARITY_${parity}
                        TEST_STOP_BITS=${stop}
                )
                target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_uart)
                target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
                set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
                unset(TARGET_NAME)
            endforeach()
        endforeach()
    endforeach()
endforeach()
