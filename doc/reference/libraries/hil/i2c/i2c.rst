.. include:: ../../../../substitutions.rst

#################
|I2C| HIL Library
#################

A software defined |I2C| library that allows you to control an |I2C| bus via XCore ports. |I2C| is a two-wire hardware serial interface, first developed by Philips.  The components in the library are controlled via C and can either act as |I2C| master or slave.

The library is compatible with multiple slave devices existing on the same bus. The |I2C| master component can be used by multiple tasks within the XCore device (each addressing the same or different slave devices).

The library can also be used to implement multiple |I2C| physical interfaces on a single XCore device simultaneously.

All signals are designed to comply with the timings in the |I2C| specification found here:

https://www.nxp.com/docs/en/user-guide/UM10204.pdf

Note that the following optional parts of the |I2C| specification are not supported:

- Multi-master arbitration
- 10-bit slave addressing
- General call addressing
- Software reset
- START byte
- Device ID
- Fast-mode Plus, High-speed mode, Ultra Fast-mode

|I2C| consists of two signals: a clock line (SCL) and a data line (SDA). Both these signals are open-drain and require external resistors to pull the line up if no device is driving the signal down. The correct value for the resistors can be found in the |I2C| specification.

All |I2C| functions can be accessed via the ``i2c.h`` header:

.. code-block:: c
   
   #include <i2c.h>

.. toctree::
   :maxdepth: 2
   :includehidden:

   i2c_master.rst
   i2c_slave.rst
   i2c_registers.rst

