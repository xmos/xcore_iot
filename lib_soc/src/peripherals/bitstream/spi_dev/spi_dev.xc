// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"
#include "spi_dev.h"

#include "debug_print.h"
#define DEVICE_ID 0

#define TEST_DEVICE     0
#define TEST_ASYNC_SPI  0
#define TEST_SYNC_SPI   0

#if TEST_DEVICE
unsafe
{
static unsafe void spi_dev_handler(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        client spi_master_async_if ?i_spi)
{
    uint8_t rx_buf[SPICONF_RX_BUFSIZE];
    uint8_t tx_buf[SPICONF_TX_BUFSIZE];
    uint8_t* movable prx_buf = rx_buf;
    uint8_t* movable ptx_buf = tx_buf;
    unsigned speed_in_khz;
    spi_mode_t mode;
    size_t rx_ptr;
    size_t tx_ptr;
    size_t len;

    uint8_t data_buf[SPICONF_TX_BUFSIZE];
    size_t data_len;
    uint32_t cmd;

    int transfer = 0;
    int no_rx = 0;
    int no_tx = 0;

    while (1)
    {
        if (peripheral != NULL && *peripheral != NULL) {
            if( transfer == 0 )
            {
                data_len = soc_peripheral_rx_dma_direct_xfer(*peripheral, data_buf, SPICONF_TX_BUFSIZE);

                memcpy( ptx_buf, data_buf, data_len );
//                for(int i=0; i< data_len; i++)
//                {
//                    ptx_buf[i] = data_buf[i];
//                }
                debug_printf("rxdmadirect datalen:%d\n", data_len);

                if (data_len > 0) {

                    debug_printf("ptx_buf[0] :%d\n", ptx_buf[0]);
                    debug_printf("ptx_buf[1] :%d\n", ptx_buf[1]);
                    debug_printf("ptx_buf[2] :%d\n", ptx_buf[2]);
                    debug_printf("ptx_buf[3] :%d\n", ptx_buf[3]);

                    transfer = 1;
                    i_spi.init_transfer_array_8(
                            (uint8_t*movable)move(prx_buf),
                            (uint8_t*movable)move(ptx_buf),
                            data_len);
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
            case SPI_DEV_SETUP:
                debug_printf("setup\n");
                soc_peripheral_varlist_rx(
                        ctrl_c, 2,
                        sizeof(unsigned), &speed_in_khz,
                        sizeof(spi_mode_t), &mode);

                i_spi.begin_transaction( DEVICE_ID, speed_in_khz, mode);
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

        case !isnull(data_from_dma_c) => soc_peripheral_rx_dma_ready(data_from_dma_c):
            debug_printf("rx from dma\n");
            data_len = soc_peripheral_rx_dma_xfer(data_from_dma_c, data_buf, sizeof(data_buf));

            memcpy( ptx_buf, data_buf, data_len );

            debug_printf("rxdma datalen:%d\n", data_len);

            if (data_len > 0) {
                transfer = 1;
                i_spi.init_transfer_array_8(
                        move(prx_buf),
                        move(ptx_buf),
                        data_len);
            }
            break;

        case i_spi.transfer_complete():
            debug_printf("complete\n");
            i_spi.retrieve_transfer_buffers_8(prx_buf, ptx_buf);

            debug_printf("prx_buf[0] :%d\n", prx_buf[0]);
            debug_printf("prx_buf[1] :%d\n", prx_buf[1]);
            debug_printf("prx_buf[2] :%d\n", prx_buf[2]);
            debug_printf("prx_buf[3] :%d\n", prx_buf[3]);
            i_spi.end_transaction(100);
            transfer = 0;
//            memcpy(data_buf, prx_buf, len);

//            if (peripheral != NULL && *peripheral != NULL) {
//                soc_peripheral_tx_dma_direct_xfer(*peripheral, data_buf, len);
//            } else if (!isnull(data_to_dma_c)) {
//                soc_peripheral_tx_dma_xfer(data_to_dma_c, data_buf, len);
//            }
            break;
        }
    }
}
}
#endif

#if TEST_ASYNC_SPI
static void spi_test_async_handler(
        client spi_master_async_if spi)
{
    while(1)
    {
        uint8_t indata[10];
        uint8_t outdata[10];
        uint8_t * movable inbuf = indata;
        uint8_t * movable outbuf = outdata;

        debug_printf("Sending SPI traffic (async)\n");
        delay_microseconds(30000);

        // Fill the out buffer with data
        outbuf[0] = 0xde;
        outbuf[1] = 0xad;
        outbuf[2] = 0xbe;
        outbuf[3] = 0xef;
        outbuf[4] = 0;
        outbuf[5] = 0xff;
        spi.begin_transaction(0, 125, SPI_MODE_1);

        // This call passes the buffers over to the SPI task, after
        // this the application cannot access the buffers until
        // the retrieve_transfer_buffers_8 function is called.
        spi.init_transfer_array_8(move(inbuf),
                                  move(outbuf),
                                  6);
        select {
            case spi.transfer_complete():
               spi.retrieve_transfer_buffers_8(inbuf, outbuf);
               break;
        }

        spi.end_transaction(100);

        delay_microseconds(30000);
    }
}
#endif

#if TEST_SYNC_SPI
static void spi_test_handler(
        client spi_master_if spi)
{
    while(1)
    {
        uint8_t inbuf[10];
        uint8_t outbuf[10];

        debug_printf("Sending SPI traffic (sync)\n");
        delay_microseconds(30000);

        // Fill the out buffer with data
        outbuf[0] = 0xde;
        outbuf[1] = 0xad;
        outbuf[2] = 0xbe;
        outbuf[3] = 0xef;
        outbuf[4] = 0;
        outbuf[5] = 0xff;
        spi.begin_transaction(0, 125, SPI_MODE_1);

        for(int i=0; i< 6; i++)
        {
            inbuf[i] = spi.transfer8(outbuf[i]);
        }

        spi.end_transaction(100);
        delay_microseconds(30000);
    }
}
#endif

void spi_dev(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        out buffered port:32 p_sclk,
        out buffered port:32 ?p_mosi,
        in buffered port:32 ?p_miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock ?clk0,
        clock ?clk1)
{
#if TEST_SYNC_SPI
    spi_master_if i_spi[1];
#endif
#if ( TEST_ASYNC_SPI || TEST_DEVICE )
    spi_master_async_if i_spi_a[1];
#endif

    par {
#if TEST_ASYNC_SPI
        spi_master_async(i_spi_a, 1,
                         p_sclk, p_mosi, p_miso, p_ss,
                         num_slaves,
                         clk0, clk1);

        spi_test_async_handler(i_spi_a[0]);
#endif
#if TEST_DEVICE
        spi_master_async(i_spi_a, 1,
                         p_sclk, p_mosi, p_miso, p_ss,
                         num_slaves,
                         clk0, clk1);
        unsafe
        {
            spi_dev_handler(peripheral,
                            data_to_dma_c, data_from_dma_c, ctrl_c,
                            i_spi[0]);
        }
#endif

#if TEST_SYNC_SPI
        spi_master(i_spi, 1,
                p_sclk, p_mosi, p_miso, p_ss,
                num_slaves, clk0);
        spi_test_handler(i_spi[0]);
#endif
    }
}
