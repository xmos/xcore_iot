cmake_minimum_required(VERSION 3.20)

## Import hil source
set(USE_I2C_HIL TRUE)
set(USE_I2S_HIL FALSE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

## If XCORE_SDK_PATH is not already defined, then we're running this test 
## independently. Set XCORE_SDK_PATH to 4 parents upwards, set the toolchain, 
## and set the project directive.
if(NOT DEFINED XCORE_SDK_PATH)
    set(XCORE_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
    cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
    cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
    cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
    cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
    include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")
    project(i2c_master_test)
endif()

include("${XCORE_SDK_PATH}/modules/hil/hil.cmake")

set(APP_NAME i2c_master_test)

set(APP_COMPILER_FLAGS
    "-O2"
    "-g"
    "-report"
    "-target=XCORE-AI-EXPLORER"
)

set(APP_SOURCES
    "src/main.c"
)

set(APP_INCLUDES
    "src"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
)

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

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

if(NOT DEFINED ENV{ACK_TEST})
    foreach(port_setup ${PORT_SETUPS})
        foreach(speed ${SPEEDS})
            foreach(stop ${STOPS})
                set(TARGET_NAME_NO_EXT "${APP_NAME}_rx_tx_${speed}_${stop}_${port_setup}")
                set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

                add_executable(${TARGET_NAME})

                target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2C_HIL_SOURCES})
                target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2C_HIL_INCLUDES})

                target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                target_compile_definitions(${TARGET_NAME}
                                        PRIVATE
                                            SPEED=${speed}
                                            STOP=${${stop}_val}
                                            PORT_SETUP=${port_setup}
                                            ENABLE_TX=1
                                            ENABLE_RX=1
                                            )

                target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
            endforeach()
        endforeach()
    endforeach()
else()
    foreach(stop ${STOPS})
        set(TARGET_NAME_NO_EXT "${APP_NAME}_tx_only_${stop}")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")
        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2C_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2C_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                PRIVATE
                                    SPEED=400
                                    STOP=${${stop}_val}
                                    PORT_SETUP=0
                                    ENABLE_TX=1
                                    ENABLE_RX=0
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
    endforeach()
endif()