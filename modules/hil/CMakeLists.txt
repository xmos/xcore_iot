## TODO place libraries in respective repos


file(GLOB_RECURSE LIB_CLOCK_CONTROL_SOURCES lib_clock_control/src/*.c)
set(LIB_CLOCK_CONTROL_INCLUDES lib_clock_control/api)
add_library(lib_clock_control STATIC EXCLUDE_FROM_ALL ${LIB_CLOCK_CONTROL_SOURCES})
target_include_directories(lib_clock_control PUBLIC ${LIB_CLOCK_CONTROL_INCLUDES})
# target_link_libraries(lib_clock_control "")
# target_compile_options(lib_clock_control PRIVATE ${COMPILE_FLAGS} )


file(GLOB_RECURSE LIB_I2C_SOURCES lib_i2c/src/*.c)
set(LIB_I2C_INCLUDES lib_i2c/api)
add_library(lib_i2c STATIC EXCLUDE_FROM_ALL ${LIB_I2C_SOURCES})
target_include_directories(lib_i2c PUBLIC ${LIB_I2C_INCLUDES})


file(GLOB_RECURSE LIB_I2S_SOURCES lib_i2s/src/*.c)
set(LIB_I2S_INCLUDES lib_i2s/api)
add_library(lib_i2s STATIC EXCLUDE_FROM_ALL ${LIB_I2S_SOURCES})
target_include_directories(lib_i2s PUBLIC ${LIB_I2S_INCLUDES})


file(GLOB_RECURSE LIB_L2_CACHE_SOURCES lib_l2_cache/src/*.c)
file(GLOB_RECURSE LIB_L2_CACHE_ASM_SOURCES lib_l2_cache/src/*.S)
set_source_files_properties(${LIB_L2_CACHE_ASM_SOURCES} PROPERTIES LANGUAGE ASM)
set(LIB_L2_CACHE_INCLUDES lib_l2_cache/lib_l2_cache/api)
add_library(lib_l2_cache STATIC EXCLUDE_FROM_ALL ${LIB_L2_CACHE_SOURCES} ${LIB_L2_CACHE_ASM_SOURCES})
target_include_directories(lib_l2_cache PUBLIC ${LIB_L2_CACHE_INCLUDES})


file(GLOB_RECURSE LIB_MIC_ARRAY_SOURCES lib_mic_array/src/*.c)
file(GLOB_RECURSE LIB_MIC_ARRAY_ASM_SOURCES lib_mic_array/src/*.S)
set_source_files_properties(${LIB_MIC_ARRAY_ASM_SOURCES} PROPERTIES LANGUAGE ASM)
set(LIB_MIC_ARRAY_INCLUDES lib_mic_array/lib_l2_cache/api)
add_library(lib_mic_array STATIC EXCLUDE_FROM_ALL ${LIB_MIC_ARRAY_SOURCES} ${LIB_MIC_ARRAY_ASM_SOURCES})
target_include_directories(lib_mic_array PUBLIC ${LIB_MIC_ARRAY_INCLUDES})


file(GLOB_RECURSE LIB_QSPI_IO_SOURCES lib_qspi_io/src/*.c)
set(LIB_QSPI_IO_INCLUDES lib_qspi_io/api)
add_library(lib_qspi_io STATIC EXCLUDE_FROM_ALL ${LIB_QSPI_IO_SOURCES})
target_include_directories(lib_qspi_io PUBLIC ${LIB_QSPI_IO_INCLUDES})


file(GLOB_RECURSE LIB_SPI_SOURCES lib_spi/src/*.c)
set(LIB_SPI_INCLUDES lib_spi/api)
add_library(lib_spi STATIC EXCLUDE_FROM_ALL ${LIB_SPI_SOURCES})
target_include_directories(lib_spi PUBLIC ${LIB_SPI_INCLUDES})


file(GLOB_RECURSE LIB_XUD_SOURCES lib_xud/lib_xud/src/*.c)
file(GLOB_RECURSE LIB_XUD_ASM_SOURCES lib_xud/lib_xud/src/*.S)
set(LIB_XUD_INCLUDES lib_xud/lib_xud/api lib_xud/lib_xud/src/user lib_xud/lib_xud/src/core)
add_library(lib_xud STATIC EXCLUDE_FROM_ALL ${LIB_XUD_SOURCES})
target_include_directories(lib_xud PUBLIC ${LIB_XUD_INCLUDES})
