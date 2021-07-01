cmake_minimum_required(VERSION 3.14)

## Import hil source
set(USE_I2C_HIL TRUE)
set(USE_I2S_HIL FALSE)
set(USE_SPI_HIL FALSE)
set(USE_QSPI_IO_HIL FALSE)
set(USE_MIC_ARRAY_HIL FALSE)
set(USE_XUD_HIL FALSE)

include("$ENV{XCORE_SDK_PATH}/modules/hil/hil.cmake")

set(APP_NAME i2c_master_reg_test)

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

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

set(TARGET_NAME_NO_EXT "${APP_NAME}")
set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")
add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2C_HIL_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2C_HIL_INCLUDES})

target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})

target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR})
