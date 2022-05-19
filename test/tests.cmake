
## Add HIL tests
include(${CMAKE_CURRENT_LIST_DIR}/hil/lib_i2c/lib_i2c.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/hil/lib_i2s/lib_i2s.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/hil/lib_qspi_io/lib_qspi_io.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/hil/lib_spi/lib_spi.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/hil/lib_uart/lib_uart.cmake)

## Add rtos drivers system tests
include(${CMAKE_CURRENT_LIST_DIR}/rtos_drivers/clock_control/clock_control.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rtos_drivers/hil/hil.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rtos_drivers/hil_add/hil_add.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rtos_drivers/usb/usb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/rtos_drivers/wifi/wifi.cmake)
