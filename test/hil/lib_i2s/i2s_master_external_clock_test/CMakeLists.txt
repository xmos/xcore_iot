cmake_minimum_required(VERSION 3.20)
project(i2s_master_external_clock_test)

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

set(APP_NAME i2s_master_external_clock_test)

set(APP_COMPILER_FLAGS
    "-O3"
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

if(NOT DEFINED ENV{NUMS_IN_OUT})
    set(NUMS_IN_OUT "4;4" "1;1" "0:4" "4:0")
else()
    set(NUMS_IN_OUT $ENV{NUMS_IN_OUT})
endif()

if(NOT DEFINED ENV{TEST_LEVEL})
    set(SMOKE 1)
else()
    if($ENV{TEST_LEVEL} STREQUAL "smoke")
        set(SMOKE 1)
    elseif($ENV{TEST_LEVEL} STREQUAL "nightly")
        set(SMOKE 0)
    endif()
endif()

if(SMOKE EQUAL 1)
    set(TEST_LEVEL "smoke")
else()
    set(TEST_LEVEL "nightly")
endif()

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#**********************
# Setup targets
#**********************

list(LENGTH NUMS_IN_OUT NUMS_IN_OUT_LEN)
math(EXPR num_pairs "${NUMS_IN_OUT_LEN} / 2")

foreach(i RANGE 1 ${num_pairs})
    list(POP_FRONT NUMS_IN_OUT num_in num_out)
    
    set(TARGET_NAME_NO_EXT "${APP_NAME}_${TEST_LEVEL}_${num_in}_${num_out}")
    set(TARGET_NAME "${TARGET_NAME_NO_EXT}.xe")

    add_executable(${TARGET_NAME})

    target_sources(${TARGET_NAME} PRIVATE ${APP_SOURCES} ${I2S_HIL_SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE ${APP_INCLUDES} ${I2S_HIL_INCLUDES})

    target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    target_compile_definitions(${TARGET_NAME}
                                PRIVATE
                                    NUM_OUT=${num_out}
                                    NUM_IN=${num_in}
                                    SMOKE=${SMOKE}
                                )

    target_link_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
    install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_DIR}/${TARGET_NAME_NO_EXT})
endforeach()
