.. include:: ../../../substitutions.rst

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

***********
Master Mode
***********

Master Usage
============

The following code snippet demonstrates the basic usage of an |I2C| master device.

.. code-block:: c

   #include <xs1.h>
   #include <i2c.h>

   i2c_master_t i2c_ctx;

   port_t p_scl = XS1_PORT_1A;
   port_t p_sda = XS1_PORT_1B;

   uint8_t data[1] = {0x99};

   // Initialize the master
   i2c_master_init(
               &i2c_ctx,
               p_scl, 0, 0,
               p_sda, 0, 0,
               100);

   // Write some data
   i2c_master_write(&i2c_ctx, 0x33, data, 1, NULL, 1);

   // Shutdown
   i2c_master_shutdown(&i2c_ctx) ;

Master API
==========

The following structures and functions are used to initialize and start an |I2C| master instance.

.. doxygengroup:: hil_i2c_master
   :content-only:

**********
Slave Mode
**********

Slave Usage
===========

The following code snippet demonstrates the basic usage of an |I2C| slave device.

.. code-block:: c

   #include <xs1.h>
   #include <i2c.h>

   port_t p_scl = XS1_PORT_1A;
   port_t p_sda = XS1_PORT_1B;

   // Setup callbacks
   //  NOTE: See API or SDK examples for more on using the callbacks
   i2c_callback_group_t i_i2c = {
        .ack_read_request = (ack_read_request_t) i2c_ack_read_req,
        .ack_write_request = (ack_write_request_t) i2c_ack_write_req,
        .master_requires_data = (master_requires_data_t) i2c_master_req_data,
        .master_sent_data = (master_sent_data_t) i2c_master_sent_data,
        .stop_bit = (stop_bit_t) i2c_stop_bit,
        .shutdown = (shutdown_t) i2c_shutdown,
        .app_data = NULL,
   };

   // Start the slave device in this thread
   //  NOTE: You may wish to launch the slave device in a different thread.  
   //        See the XTC Tools documentation reference for lib_xcore.
   i2c_slave(&i_i2c, p_scl, p_sda, 0x3c);

Slave API
=========

The following structures and functions are used to initialize and start an |I2C| slave instance.

.. doxygengroup:: hil_i2c_slave
   :content-only:

*********
Registers
*********

Register API
============

The following structures and functions are used to read and write |I2C| registers.

.. doxygengroup:: hil_i2c_register
   :content-only: