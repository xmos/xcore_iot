.. include:: ../../../substitutions.rst

#################
|I2S| RTOS Driver
#################

This driver can be used to instantiate and control an |I2S| master or slave mode I/O interface on XCore in an RTOS application.

******************
Initialization API
******************
The following structures and functions are used to initialize and start an |I2S| driver instance.

.. toctree::
   :maxdepth: 2
   :includehidden:

   i2s_master.rst
   i2s_slave.rst

.. doxygengroup:: rtos_i2s_driver
   :content-only:

********
Core API
********

The following functions are the core |I2S| driver functions that are used after it has been initialized and started.

.. doxygengroup:: rtos_i2s_driver_core
   :content-only:

**********************
RPC Initialization API
**********************

The following functions may be used to share a |I2S| driver instance with other XCore tiles. Tiles that the
driver instance is shared with may call any of the core functions listed above.

.. doxygengroup:: rtos_i2s_driver_rpc
   :content-only:
