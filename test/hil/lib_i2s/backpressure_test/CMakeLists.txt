cmake_minimum_required(VERSION 3.14)

## Import hil source
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL TRUE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

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

set(SAMPLE_RATES 768000 384000 192000)
set(CHANS 1 2 3 4)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

foreach(rate ${SAMPLE_RATES})
    foreach(chan ${CHANS})
        set(TARGET_NAME_NO_EXT "${APP_NAME}_${rate}_${chan}_5_5")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                   PRIVATE
                                       SAMPLE_FREQUENCY=${rate}
                                       NUM_I2S_LINES=${chan}
                                       RECEIVE_DELAY_INCREMENT=5
                                       SEND_DELAY_INCREMENT=5
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})


        set(TARGET_NAME_NO_EXT "${APP_NAME}_${rate}_${chan}_10_0")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                   PRIVATE
                                       SAMPLE_FREQUENCY=${rate}
                                       NUM_I2S_LINES=${chan}
                                       RECEIVE_DELAY_INCREMENT=10
                                       SEND_DELAY_INCREMENT=0
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})


        set(TARGET_NAME_NO_EXT "${APP_NAME}_${rate}_${chan}_0_10")
        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

        add_executable(${TARGET_NAME})

        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        target_compile_definitions(${TARGET_NAME}
                                   PRIVATE
                                       SAMPLE_FREQUENCY=${rate}
                                       NUM_I2S_LINES=${chan}
                                       RECEIVE_DELAY_INCREMENT=0
                                       SEND_DELAY_INCREMENT=10
                                    )

        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
    endforeach()
endforeach()
