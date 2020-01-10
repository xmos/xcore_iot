// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"
#include "spi_dev.h"

#include "debug_print.h"
#define DEVICE_ID 0

// TODO: move to bitstream.xc
spi_fast_ports spi_ctx = {
        PORT_EXPANSION_1,
        PORT_EXPANSION_5,
        PORT_EXPANSION_7,
        PORT_EXPANSION_3,
        on tile[0]: XS1_CLKBLK_1,
};

static void spi_test_fast_handler(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c)
{
    uint8_t rx_buf[SPICONF_RX_BUFSIZE];
    uint8_t tx_buf[SPICONF_TX_BUFSIZE];
    uint8_t* prx_buf = rx_buf;
    uint8_t* ptx_buf = tx_buf;
    unsigned speed_in_khz;
    size_t rx_ptr;
    size_t tx_ptr;
    size_t len;

    uint8_t data_buf[SPICONF_TX_BUFSIZE];
    size_t data_len;
    uint32_t cmd;

    int no_rx = 0;
    int no_tx = 0;

    while (1)
    {
        if (peripheral != NULL && *peripheral != NULL)
        {
            data_len = soc_peripheral_rx_dma_direct_xfer(*peripheral, data_buf, SPICONF_TX_BUFSIZE);

            memcpy( ptx_buf, data_buf, data_len );
            debug_printf("rxdmadirect datalen:%d\n", data_len);

            if (data_len > 0)
            {
                debug_printf("ptx_buf[0] :%d\n", ptx_buf[0]);
                debug_printf("ptx_buf[1] :%d\n", ptx_buf[1]);
                debug_printf("ptx_buf[2] :%d\n", ptx_buf[2]);
                debug_printf("ptx_buf[3] :%d\n", ptx_buf[3]);

                spi_fast(data_len, ptx_buf, spi_ctx, SPI_READ_WRITE);

                if( !no_rx )
                {
                    debug_printf("rx to dma\n");
                    debug_printf("prx_buf[0] :%d\n", ptx_buf[0]);
                    debug_printf("prx_buf[1] :%d\n", ptx_buf[1]);
                    debug_printf("prx_buf[2] :%d\n", ptx_buf[2]);
                    debug_printf("prx_buf[3] :%d\n", ptx_buf[3]);
                    if (peripheral != NULL && *peripheral != NULL) {
                        soc_peripheral_tx_dma_direct_xfer(
                                *peripheral,
                                ptx_buf,
                                data_len);
                    } else if (!isnull(data_to_dma_c)) {
                        soc_peripheral_tx_dma_xfer(
                                data_to_dma_c,
                                ptx_buf,
                                data_len);
                    }
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
            case SPI_DEV_INIT:
                debug_printf("init\n");
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

            case SPI_DEV_TRANSACTION:
                debug_printf("transaction\n");
                soc_peripheral_varlist_rx(
                        ctrl_c, 3,
                        sizeof(size_t), &len,
                        4, &rx_ptr,     // sizeof(unsafe uint8_t*)
                        4, &tx_ptr);    // sizeof(unsafe uint8_t*)

                if( rx_ptr == NULL )
                {
                    no_rx = 1;
                    debug_printf("tx only\n");
                    ;   // we are in tx only
                }

                if( tx_ptr == NULL )
                {
                    no_tx = 1;
                    debug_printf("rx only\n");
                    ;   // we are in rx only
                }

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

//        case i_spi.transfer_complete():
//            debug_printf("complete\n");
//            i_spi.retrieve_transfer_buffers_8(prx_buf, ptx_buf);
//
//            debug_printf("prx_buf[0] :%d\n", prx_buf[0]);
//            debug_printf("prx_buf[1] :%d\n", prx_buf[1]);
//            debug_printf("prx_buf[2] :%d\n", prx_buf[2]);
//            debug_printf("prx_buf[3] :%d\n", prx_buf[3]);
//            i_spi.end_transaction(100);
////            memcpy(data_buf, prx_buf, len);
//
////            if (peripheral != NULL && *peripheral != NULL) {
////                soc_peripheral_tx_dma_direct_xfer(*peripheral, data_buf, len);
////            } else if (!isnull(data_to_dma_c)) {
////                soc_peripheral_tx_dma_xfer(data_to_dma_c, data_buf, len);
////            }
//            break;
        }
    }
}

void spi_dev(
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
