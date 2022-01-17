.. include:: ../../../../substitutions.rst

**********
SPI Master
**********

SPI Master Usage
================

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

SPI Master API
==============

The following structures and functions are used to initialize and start an SPI master instance.

.. doxygengroup:: hil_spi_master
   :content-only:
