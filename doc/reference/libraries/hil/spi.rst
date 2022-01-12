.. include:: ../../../substitutions.rst

###############
SPI HIL Library
###############

A software defined SPI (serial peripheral interface) library that allows you to control a SPI bus via the XCore GPIO hardware-response ports. SPI is a four-wire hardware bi-directional serial interface. The components in the library are controlled via C and can either act as SPI master or slave.

The SPI bus can be used by multiple tasks within the XCore device and (each addressing the same or different slaves) and is compatible with other slave devices on the same bus.

The SPI protocol requires a clock, one or more slave selects and either one or two data wires.

.. _spi_wire_table:

.. list-table:: SPI data wires
     :class: vertical-borders horizontal-borders

     * - *SCLK*
       - Clock line, driven by the master
     * - *MOSI*
       - Master Output, Slave Input data line, driven by the master
     * - *MISO*
       - Master Input, Slave Output data line, driven by the slave
     * - *SS*
       - Slave select line, driven by the master

All SPI functions can be accessed via the ``spi.h`` header:

.. code-block:: c
   
   #include <spi.h>

***********
Master Mode
***********

Master Usage
============

The following code snippet demonstrates the basic usage of an SPI master device.

.. code-block:: c

   #include <xs1.h>
   #include <spi.h>

   spi_master_t spi_ctx;
   spi_master_device_t spi_dev;

   port_t p_miso = XS1_PORT_1A;
   port_t p_ss[1] = {XS1_PORT_1B};
   port_t p_sclk = XS1_PORT_1C;
   port_t p_mosi = XS1_PORT_1D;
   xclock_t cb = XS1_CLKBLK_1;

   uint8_t tx[4] = {0x01, 0x02, 0x04, 0x08};
   uint8_t rx[4];

   // Initialize the master device
   spi_master_init(&spi_ctx, cb, p_ss[0], p_sclk, p_mosi, p_miso);
   spi_master_device_init(&spi_dev, &spi_ctx,
        1,
        SPI_MODE_0,
        spi_master_source_clock_ref,
        0,
        spi_master_sample_delay_0,
        0, 0 ,0 ,0 );

   // Transfer some data
   spi_master_start_transaction(&spi_ctx);
   spi_master_transfer(&spi_ctx, (uint8_t *)tx, (uint8_t *)rx, 4);
   spi_master_end_transaction(&spi_ctx);

Master API
==========

The following structures and functions are used to initialize and start an SPI master instance.

.. doxygengroup:: hil_spi_master
   :content-only:

**********
Slave Mode
**********

Slave Usage
===========

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

Slave API
=========

The following structures and functions are used to initialize and start an SPI slave instance.

.. doxygengroup:: hil_spi_slave
   :content-only:

