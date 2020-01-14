// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"
#include "spi_master_dev.h"

#include "debug_print.h"
#define DEVICE_ID 0

// TODO: move to bitstream.xc
spi_fast_ports spi_ctx = {
        PORT_EXPANSION_5,
        PORT_EXPANSION_3,
        PORT_EXPANSION_1,
        PORT_EXPANSION_7,
        on tile[0]: XS1_CLKBLK_1,
};

static void spi_test_fast_handler(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c)
{
    size_t rx_ptr;
    size_t tx_ptr;

    uint8_t data_buf[SPICONF_TX_BUFFER];
    size_t data_len;
    uint32_t cmd;

    size_t rx_len = 0;

    while(1)
    {
        if(peripheral != NULL && *peripheral != NULL)
        {
            /* Check that a valid buffer was set */
            if( ( data_len = soc_peripheral_rx_dma_direct_xfer(*peripheral, data_buf, SPICONF_TX_BUFFER) ) >= 0 )
            {
                if(data_len > 0)    /* TX and RX */
                {
                    spi_fast(data_len, data_buf, spi_ctx, SPI_READ_WRITE);
                }
                else    /* RX "only" */
                {
                    if(rx_len > 0)
                    {
                        memset(data_buf, SPICONF_RX_ONLY_CHAR, rx_len);
                        spi_fast(rx_len, data_buf, spi_ctx, SPI_READ_WRITE);
                    }
                }

                if(rx_len > 0)  /* Send response if it had been requested */
                {
                    if (peripheral != NULL && *peripheral != NULL) {
                        soc_peripheral_tx_dma_direct_xfer(
                                *peripheral,
                                data_buf,
                                rx_len);
                    } else if (!isnull(data_to_dma_c)) {
                        soc_peripheral_tx_dma_xfer(
                                data_to_dma_c,
                                data_buf,
                                rx_len);
                    }
                    rx_len = 0;
                }
            }
        }

        select
        {
        case !isnull(ctrl_c) => soc_peripheral_function_code_rx(ctrl_c, &cmd):
            switch (cmd)
            {
            case SOC_PERIPHERAL_DMA_TX:
                /*
                 * The application has added a new DMA TX buffer. This
                 * ensures that this select statement wakes up and gets
                 * the TX data in the code above.
                 */
                break;
            case SPI_MASTER_DEV_INIT:
                soc_peripheral_varlist_rx(
                        ctrl_c, 6,
                        sizeof(unsigned), &spi_ctx.cs_port_bit,
                        sizeof(unsigned), &spi_ctx.cpol,
                        sizeof(unsigned), &spi_ctx.cpha,
                        sizeof(unsigned), &spi_ctx.clock_divide,
                        sizeof(unsigned), &spi_ctx.cs_to_data_delay_ns,
                        sizeof(unsigned), &spi_ctx.byte_setup_ns);

                spi_fast_init(spi_ctx);
                break;

            case SPI_MASTER_DEV_TRANSACTION:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(size_t), &rx_len);
                break;

            default:
                /* Command is not valid */
                fail("Invalid cmd");
                break;
            }
            break;

//        case !isnull(data_from_dma_c) => soc_peripheral_rx_dma_ready(data_from_dma_c):
//            debug_printf("rx from dma\n");
//            data_len = soc_peripheral_rx_dma_xfer(data_from_dma_c, data_buf, sizeof(data_buf));
//
//            memcpy( ptx_buf, data_buf, data_len );
//
//            debug_printf("rxdma datalen:%d\n", data_len);
//
//            if (data_len > 0) {
////                spi_fast(data_len, ptx_buf, spi_ctx, SPI_READ_WRITE);
//            }
//            break;

        }
    }
}

void spi_master_dev(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c)
{
    par {
        spi_test_fast_handler(peripheral,
                              data_to_dma_c, data_from_dma_c, ctrl_c
                              );
    }
}
