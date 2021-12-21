.. include:: ../../../substitutions.rst

#################
|I2C| HIL Library
#################

This library can be used to instantiate and control an |I2C| I/O interface on XCore.  Both master and slave modes are supported.

***************
Master Mode API
***************

The following structures and functions are used to initialize and start an |I2C| master instance.

.. doxygengroup:: hil_i2c_master
   :content-only:

**************
Slave Mode API
**************

The following structures and functions are used to initialize and start an |I2C| slave instance.

.. doxygengroup:: hil_i2c_slave
   :content-only:
