
add_subdirectory(lib_clock_control)
add_subdirectory(lib_i2c)
add_subdirectory(lib_i2s)
add_subdirectory(lib_l2_cache)
add_subdirectory(lib_mic_array)
add_subdirectory(lib_qspi_io)
add_subdirectory(lib_spi)
add_subdirectory(lib_xud)

## Create an aggregate of hil libraries
add_library(xcore_sdk_hil INTERFACE)
target_link_libraries(xcore_sdk_hil
    INTERFACE
        sdk::hil::lib_clock_control
        sdk::hil::lib_i2c
        sdk::hil::lib_i2s
        sdk::hil::lib_l2_cache
        sdk::hil::lib_mic_array
        sdk::hil::lib_qspi_io
        sdk::hil::lib_spi
        sdk::hil::lib_xud
)
add_library(sdk::hil ALIAS xcore_sdk_hil)
