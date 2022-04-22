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
if(NOT DEFINED ENV{PORT_SETUPS})
    set(PORT_SETUPS 0 1 2 3 4)
else()
    set(PORT_SETUPS $ENV{PORT_SETUPS})
endif()
if(NOT DEFINED ENV{SPEEDS})
    set(SPEEDS 10 100 400)
else()
    set(SPEEDS $ENV{SPEEDS})
endif()
if(NOT DEFINED ENV{STOPS})
    set(STOPS stop no_stop)
else()
    set(STOPS $ENV{STOPS})
endif()

set(stop_val 1)
set(no_stop_val 0)

#**********************
# Setup targets
#**********************
foreach(port_setup ${PORT_SETUPS})
    foreach(speed ${SPEEDS})
        foreach(stop ${STOPS})
            set(TARGET_NAME "test_hil_i2c_master_test_${speed}_${stop}_${port_setup}")
            add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
            target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
            target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
            target_compile_definitions(${TARGET_NAME}
                PRIVATE
                    ${APP_COMPILE_DEFINITIONS}
                    SPEED=${speed}
                    STOP=${${stop}_val}
                    PORT_SETUP=${port_setup}
                    ENABLE_TX=1
                    ENABLE_RX=1
            )
            target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
            target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_i2c)
            target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
            set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
            unset(TARGET_NAME)
        endforeach()
    endforeach()
endforeach()

foreach(stop ${STOPS})
    set(TARGET_NAME "test_hil_i2c_master_test_tx_only_${stop}")
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
    target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
    target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
    target_compile_definitions(${TARGET_NAME}
        PRIVATE
            ${APP_COMPILE_DEFINITIONS}
            SPEED=400
            STOP=${${stop}_val}
            PORT_SETUP=0
            ENABLE_TX=1
            ENABLE_RX=0
    )
    target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    target_link_libraries(${TARGET_NAME} PUBLIC sdk::hil::lib_i2c)
    target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
    unset(TARGET_NAME)
endforeach()
