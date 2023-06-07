set(TARGET_FILE ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn)
set(XSCOPE_PORT localhost:12345)

#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.xc 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c 
)

set(APP_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
)

enable_language(CXX C)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
    ${TARGET_FILE}
    -DTF_LITE_STATIC_MEMORY
    -DXCORE
  # -DNDEBUG                        # define this to remove debug and profiling
  # -DTF_LITE_STRIP_ERROR_STRINGS   # define this to remove logging
)

set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_SUPPORTS_TILE_0=1
    PLATFORM_SUPPORTS_TILE_1=1
    PLATFORM_SUPPORTS_TILE_2=0
    PLATFORM_SUPPORTS_TILE_3=0
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    QSPI_FLASH_FILESYSTEM_START_ADDRESS=${FILESYSTEM_START_ADDRESS}
    QSPI_FLASH_MODEL_START_ADDRESS=${MODEL_START_ADDRESS}
    QSPI_FLASH_CALIBRATION_ADDRESS=${CALIBRATION_PATTERN_START_ADDRESS}
)

set(APP_LINK_OPTIONS
    -lquadflash
    -report
    ${TARGET_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
    inferencing_tflite_micro
    lib_qspi_fast_read
)

#**********************
# Tile Targets
#**********************
add_executable(example_bare_metal_vww)
target_sources(example_bare_metal_vww PUBLIC ${APP_SOURCES})
target_include_directories(example_bare_metal_vww PUBLIC ${APP_INCLUDES})
target_compile_definitions(example_bare_metal_vww PRIVATE ${APP_COMPILE_DEFINITIONS})
target_compile_options(example_bare_metal_vww PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(example_bare_metal_vww PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(example_bare_metal_vww PRIVATE ${APP_LINK_OPTIONS})

#**********************
# Create data partition support targets
#**********************
set(FLASH_CAL_FILE ${LIB_QSPI_FAST_READ_ROOT_PATH}/lib_qspi_fast_read/calibration_pattern.bin)
set(MODEL_FILE ${CMAKE_CURRENT_LIST_DIR}/model/vww_model_nibble_swapped.bin)
set(DATA_PARTITION_FILE example_bare_metal_vww_data_partition.bin)

set(CALIBRATION_PATTERN_DATA_PARTITION_OFFSET 0)
math(EXPR MODEL_DATA_PARTITION_OFFSET
    "${LIB_QSPI_FAST_READ_DEFAULT_CAL_SIZE_BYTES}"
    OUTPUT_FORMAT HEXADECIMAL
)

add_custom_command(
    OUTPUT ${DATA_PARTITION_FILE}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${DATA_PARTITION_FILE}
    COMMAND datapartition_mkimage -v -b 1
    -i ${FLASH_CAL_FILE}:${CALIBRATION_PATTERN_DATA_PARTITION_OFFSET} ${MODEL_FILE}:${MODEL_DATA_PARTITION_OFFSET}
    -o ${DATA_PARTITION_FILE}
    DEPENDS
        ${MODEL_FILE}
        ${FLASH_CAL_FILE}
    COMMENT
        "Create data partition"
    VERBATIM
)

#**********************
# Create targets
#**********************

add_custom_target(flash_app_example_bare_metal_vww
    COMMAND xflash --force --write-all ${DATA_PARTITION_FILE} --target-file=${TARGET_FILE}
    DEPENDS
        ${DATA_PARTITION_FILE}
    COMMENT
        "Flash data partition"
)

add_custom_target(run_example_bare_metal_vww
  COMMAND xrun --xscope-realtime --xscope-port ${XSCOPE_PORT} example_bare_metal_vww.xe
  DEPENDS example_bare_metal_vww
  COMMENT
    "Run application"
  VERBATIM
)
