.. include:: ../../../../substitutions.rst

***********
|I2C| Slave
***********

|I2C| Slave Usage
=================

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

|I2C| Slave API
===============

The following structures and functions are used to initialize and start an |I2C| slave instance.

.. doxygengroup:: hil_i2c_slave
   :content-only:
