.. include:: ../../../../substitutions.rst

**********
QSPI Flash
**********

QSPI Flash Usage
================

The following code snippet demonstrates the basic usage of an QSPI flash device.

.. code-block:: c

   #include <xs1.h>
   #include "qspi_flash.h"

   qspi_flash_ctx_t qspi_flash_ctx;
   qspi_io_ctx_t *qspi_io_ctx = &qspi_flash_ctx.qspi_io_ctx;

   uint8_t data[4];

   // Setup the flash device
   qspi_flash_ctx.custom_clock_setup = 1;
   qspi_flash_ctx.quad_page_program_cmd = qspi_flash_page_program_1_4_4;
   qspi_flash_ctx.source_clock = qspi_io_source_clock_xcore;

   qspi_io_ctx->clock_block = FLASH_CLKBLK;
   qspi_io_ctx->cs_port = PORT_SQI_CS;
   qspi_io_ctx->sclk_port = PORT_SQI_SCLK;
   qspi_io_ctx->sio_port = PORT_SQI_SIO;

   // Full speed clock configuration
   qspi_io_ctx->full_speed_clk_divisor = 5; // 600 MHz / (2*5) -> 60 MHz
   qspi_io_ctx->full_speed_sclk_sample_delay = 1;
   qspi_io_ctx->full_speed_sclk_sample_edge = qspi_io_sample_edge_rising;

   // SPI read clock configuration
   qspi_io_ctx->spi_read_clk_divisor = 12;  // 600 MHz / (2*12) -> 25 MHz

   qspi_io_ctx->spi_read_sclk_sample_delay = 0;
   qspi_io_ctx->spi_read_sclk_sample_edge = qspi_io_sample_edge_falling;
   qspi_io_ctx->full_speed_sio_pad_delay = 0;
   qspi_io_ctx->spi_read_sio_pad_delay = 0;

   // Initialize the flash device
   qspi_flash_init(&qspi_flash_ctx);

   // Read some data
   qspi_flash_read(&qspi_flash_ctx, *data, 0x64, 4);

QSPI Flash API
==============

The following structures and functions are used to QSPI flash I/O.

.. doxygengroup:: hil_qspi_flash
   :content-only:
