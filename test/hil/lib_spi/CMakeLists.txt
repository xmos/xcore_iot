set(lib_spi_master_tester_includes "${CMAKE_CURRENT_SOURCE_DIR}/lib_spi_master_tester/src")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spi_master_sync_multi_device)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spi_master_sync_rx_tx)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/spi_slave_rx_tx)
