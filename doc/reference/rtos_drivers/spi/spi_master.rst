######################
SPI Master RTOS Driver
######################

This driver can be used to instantiate and control a SPI master I/O interface on XCore in an RTOS application.

*****************************
SPI Master Initialization API
*****************************
The following structures and functions are used to initialize and start a SPI master driver instance.

.. doxygengroup:: rtos_spi_master_driver
   :content-only:

*******************
SPI Master Core API
*******************

The following functions are the core SPI master driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_spi_master_driver_core
   :content-only:

*********************************
SPI Master RPC Initialization API
*********************************

The following functions may be used to share a SPI master driver instance with other XCore tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_spi_master_driver_rpc
   :content-only:
