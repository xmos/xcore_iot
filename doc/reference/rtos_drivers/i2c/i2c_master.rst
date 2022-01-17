.. include:: ../../../substitutions.rst

########################
|I2C| Master RTOS Driver
########################

This driver can be used to instantiate and control an |I2C| master I/O interface on XCore in an RTOS application.

*******************************
|I2C| Master Initialization API
*******************************

The following structures and functions are used to initialize and start an |I2C| driver instance.

.. doxygengroup:: rtos_i2c_master_driver
   :content-only:

*********************
|I2C| Master Core API
*********************

The following functions are the core |I2C| driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_i2c_master_driver_core
   :content-only:

***********************************
|I2C| Master RPC Initialization API
***********************************

The following functions may be used to share a |I2C| driver instance with other XCore tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_i2c_master_driver_rpc
   :content-only:

