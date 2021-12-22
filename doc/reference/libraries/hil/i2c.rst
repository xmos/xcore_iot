.. include:: ../../../substitutions.rst

#################
|I2C| HIL Library
#################

A software defined |I2C| library that allows you to control an |I2C| bus via XCore ports. |I2C| is a two-wire hardware serial interface, first developed by Philips.  The components in the library are controlled via C and can either act as |I2C| master or slave.

The library is compatible with multiple slave devices existing on the same bus. The |I2C| master component can be used by multiple tasks within the XCore device (each addressing the same or different slave devices).

The library can also be used to implement multiple |I2C| physical interfaces on a single XCore device simultaneously.

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

************
Register API
************

The following structures and functions are used to read and write |I2C| registers.

.. doxygengroup:: hil_i2c_register
   :content-only: