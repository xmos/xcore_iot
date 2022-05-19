
add_subdirectory(lib_clock_control)
add_subdirectory(lib_i2c)
add_subdirectory(lib_i2s)
add_subdirectory(lib_l2_cache)
add_subdirectory(lib_mic_array)
add_subdirectory(lib_qspi_io)
add_subdirectory(lib_spi)
add_subdirectory(lib_xud)
add_subdirectory(lib_uart)

if((${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A) OR (${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS2A))
    ## Create an aggregate of hil libraries
    add_library(xcore_sdk_hil INTERFACE)
    target_link_libraries(xcore_sdk_hil
        INTERFACE
            sdk::hil::lib_clock_control
            sdk::hil::lib_i2c
            $<$<PLATFORM_ID:XCORE_XS3A>:sdk::hil::lib_l2_cache>
            sdk::hil::lib_qspi_io
            sdk::hil::lib_spi
            sdk::hil::lib_xud
            sdk::hil::lib_uart
    )
    add_library(sdk::hil ALIAS xcore_sdk_hil)

    ## Create an aggregate of hil libraries
    add_library(xcore_sdk_hil_audio INTERFACE)
    target_link_libraries(xcore_sdk_hil_audio
        INTERFACE
            sdk::hil::lib_mic_array
            sdk::hil::lib_i2s
    )
    add_library(sdk::hil_audio ALIAS xcore_sdk_hil_audio)
endif()
