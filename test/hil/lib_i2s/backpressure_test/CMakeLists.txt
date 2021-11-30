cmake_minimum_required(VERSION 3.20)
project(backpressure_test)

## Import hil source
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL TRUE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("${XCORE_SDK_PATH}/modules/hil/hil.cmake")

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

set(APP_NAME backpressure_test)

set(APP_COMPILER_FLAGS
    "-O2"
    "-g"
    "-report"
    "-target=XCORE-AI-EXPLORER"
    "-fxscope"
)

set(APP_SOURCES
    "src/main.c"
    "src/config.xscope"
)

set(APP_INCLUDES
    "src"
)

add_compile_definitions(
    DEBUG_PRINT_ENABLE=1
    SIM=1
    GENERATE_MCLK=1
)

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

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************


list(LENGTH RX_TX_INCS RX_TX_INCS_LEN)
math(EXPR num_inc_pairs "${RX_TX_INCS_LEN} / 2")

foreach(i RANGE 1 ${num_inc_pairs})
    list(POP_FRONT RX_TX_INCS rx_inc tx_inc)
    message("${rx_inc} ${tx_inc}")
    foreach(rate ${SAMPLE_RATES})
        foreach(chan ${CHANS})
            set(TARGET_NAME_NO_EXT "${APP_NAME}_${rate}_${chan}_${rx_inc}_${tx_inc}")
            set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

            add_executable(${TARGET_NAME})

            target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
            target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

            target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
            target_compile_definitions(${TARGET_NAME}
                                    PRIVATE
                                        SAMPLE_FREQUENCY=${rate}
                                        NUM_I2S_LINES=${chan}
                                        RECEIVE_DELAY_INCREMENT=${rx_inc}
                                        SEND_DELAY_INCREMENT=${tx_inc}
                                        )

            target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
            install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
        endforeach()
    endforeach()
endforeach()