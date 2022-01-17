.. include:: ../../../../substitutions.rst

*********
SPI Slave
*********

SPI Slave Usage
===============

The following code snippet demonstrates the basic usage of an SPI slave device.

.. code-block:: c

   #include <xs1.h>
   #include <spi.h>

   // Setup callbacks
   //  NOTE: See API or SDK examples for more on using the callbacks
   spi_slave_callback_group_t spi_cbg = {
        .slave_transaction_started = (slave_transaction_started_t) start,
        .slave_transaction_ended = (slave_transaction_ended_t) end,
        .app_data = NULL
   };

   port_t p_miso = XS1_PORT_1A;
   port_t p_cs   = XS1_PORT_1B;
   port_t p_sclk = XS1_PORT_1C;
   port_t p_mosi = XS1_PORT_1D;
   xclock_t cb   = XS1_CLKBLK_1;

   // Start the slave device in this thread
   //  NOTE: You may wish to launch the slave device in a different thread.  
   //        See the XTC Tools documentation reference for lib_xcore.
   spi_slave(&spi_cbg, p_sclk, p_mosi, p_miso, p_cs, cb, SPI_MODE_0);

SPI Slave API
=============

The following structures and functions are used to initialize and start an SPI slave instance.

.. doxygengroup:: hil_spi_slave
   :content-only:

