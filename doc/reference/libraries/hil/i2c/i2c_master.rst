.. include:: ../../../../substitutions.rst

************
|I2C| Master
************

|I2C| Master Usage
==================

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

|I2C| Master API
================

The following structures and functions are used to initialize and start an |I2C| master instance.

.. doxygengroup:: hil_i2c_master
   :content-only:
