cmake_minimum_required(VERSION 3.20)
project(spi_slave_rx_tx)

## Import hil source
set(USE_I2C_HIL FALSE)
set(USE_I2S_HIL FALSE)
set(USE_SPI_HIL TRUE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

## Import XMOS configurations
include("$ENV{XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

set(APP_NAME spi_slave_rx_tx)

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
    ""
)

if(NOT DEFINED ENV{FULL_LOAD})
    set(FULL_LOAD 0 1)
else()
    set(FULL_LOAD $ENV{FULL_LOAD})
endif()

if(NOT DEFINED ENV{MISO_ENABLED})
    set(MISO_ENABLED 0 1)
else()
    set(MISO_ENABLED $ENV{MISO_ENABLED})
endif()

if(NOT DEFINED ENV{MOSI_ENABLED})
    set(MOSI_ENABLED 0 1)
else()
    set(MOSI_ENABLED $ENV{MOSI_ENABLED})
endif()

if(NOT DEFINED ENV{SPI_MODE})
    set(SPI_MODE 0 1 2 3)
else()
    set(SPI_MODE $ENV{SPI_MODE})
endif()

if(NOT DEFINED ENV{IN_PLACE})
    set(IN_PLACE 0 1)
else()
    set(IN_PLACE $ENV{IN_PLACE})
endif()

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

foreach(load ${FULL_LOAD})
    foreach(miso ${MISO_ENABLED})
        foreach(mosi ${MOSI_ENABLED})
            foreach(mode ${SPI_MODE})
                foreach(in_place ${IN_PLACE})
                    set(TARGET_NAME_NO_EXT "${APP_NAME}_${load}_${miso}_${mosi}_${mode}_${in_place}")
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
                                                   SPI_MODE=${mode}
                                                   IN_PLACE_TRANSACTION=${in_place}
                                                )

                    target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
                    install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endforeach()
