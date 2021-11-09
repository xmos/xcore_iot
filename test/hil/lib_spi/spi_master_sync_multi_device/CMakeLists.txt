cmake_minimum_required(VERSION 3.20)

## Import hil source
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL FALSE)
set(USE_SPI_HIL TRUE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

set(APP_NAME spi_master_sync_multi_device)

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
    ${lib_spi_master_tester_includes}
)

add_compile_definitions(
    ""
)

set(FULL_LOAD 0 1)
set(MISO_ENABLED 0 1)
set(MOSI_ENABLED 0 1)
set(SPI_MODE 0 1 2 3)
set(DIVS 4 8 80)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

foreach(load ${FULL_LOAD})
    foreach(miso ${MISO_ENABLED})
        foreach(mosi ${MOSI_ENABLED})
            foreach(mode ${SPI_MODE})
                foreach(div ${DIVS})
                    if(${miso} OR ${mosi})
                        set(TARGET_NAME_NO_EXT "${APP_NAME}_${load}_${miso}_${mosi}_${div}_${mode}")
                        set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

                        add_executable(${TARGET_NAME})

                        target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${SPI_HIL_SOURCES})
                        target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${SPI_HIL_INCLUDES})

                        target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                        target_compile_definitions(${TARGET_NAME}
                                                   PRIVATE
                                                       FULL_LOAD=${load}
                                                       MISO_ENABLED=${miso}
                                                       MOSI_ENABLED=${mosi}
                                                       MODE=${mode}
                                                       DIV=${div}
                                                    )

                        target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                        install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
                    endif()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endforeach()
