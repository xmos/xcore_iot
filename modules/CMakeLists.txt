

## Add frameworks
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Need to guard so host targets will not be built
    add_subdirectory(core)
endif()

add_subdirectory(io)
add_subdirectory(rtos)

## Add additional modules
add_subdirectory(qspi_fast_read)
add_subdirectory(sample_rate_conversion)
add_subdirectory(xscope_fileio)
