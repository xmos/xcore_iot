// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>
#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"

/* App headers */
#include "spi_test.h"
#include "debug_print.h"

    /* Test Params */
#define        test_interval_ms   100
#define        tx_len             8
static uint8_t test_msg[tx_len] = { 0xDE, 0xAD, 0xBE, 0xEF,
                                    0xFE, 0xED, 0xFA, 0xCE};

#define SPI_TIMING                0 /* SPI_TIMING cannot be used on app2, as all hwtimers are consumed */
#define SPI_DEBUG_PRINT           1

#if SPI_TIMING
static hwtimer_t spitimer;
static uint32_t time;
static uint32_t start;

#define SPI_TIMING_INIT()     { hwtimer_alloc(&spitimer); }
#define SPI_TIMING_START()    { hwtimer_get_time(spitimer, &start); }
#define SPI_TIMING_END()      { hwtimer_get_time(spitimer, &time);         \
                                debug_printf("transaction time: %d\n", time - start); }
#else
#define SPI_TIMING_INIT()     {}
#define SPI_TIMING_START()    {}
#define SPI_TIMING_END()      {}
#endif  /* SPI_TIMING */


static void spi_test(void *arg)
{
    soc_peripheral_t spi_dev = arg;
    uint8_t *tx_buf;
    uint8_t *rx_buf;
    int test = 0;

    spi_master_device_init(spi_dev,
                           appconfSPI_CS_PORT_BIT,
                           appconfSPI_CPOL,
                           appconfSPI_CPHA,
                           appconfSPI_CLOCKDIV,
                           appconfSPI_CS_TO_DATA_DELAY_NS,
                           appconfSPI_BYTE_SETUP_NS);

    SPI_TIMING_INIT();

    for(;;)
    {
        vTaskDelay( pdMS_TO_TICKS( test_interval_ms ) );

        switch(test++)
        {
        default:
        case 0: /* start */
            debug_printf("SPI Test Start\n");
            break;
        case 1: /* transmit only */
            debug_printf("Send Test\n");
            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);

            SPI_TIMING_START();
            spi_transaction(spi_dev,
                            NULL, 0,
                            tx_buf, tx_len);
            SPI_TIMING_END();

#if SPI_DEBUG_PRINT
            debug_printf("\tSent:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\ttx[%d]:%x\n", i, tx_buf[i]);
            }
#endif

            /* Cleanup */
            if(tx_buf != NULL) { vPortFree(tx_buf); }

            break;
        case 2: /* receive only */
            debug_printf("Receive Test\n");
            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            SPI_TIMING_START();
            spi_transaction(spi_dev,
                            rx_buf, tx_len,
                            NULL, 0);
            SPI_TIMING_END();

#if SPI_DEBUG_PRINT
            debug_printf("\tReceived:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\trx[%d]:%x\n", i, rx_buf[i]);
            }
#endif

            /* Cleanup */
            if(rx_buf != NULL) { vPortFree(rx_buf); }
            break;

        case 3: /* transaction */
            debug_printf("Transaction Test\n");
            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);

            SPI_TIMING_START();
            spi_transaction(spi_dev,
                            rx_buf, tx_len,
                            tx_buf, tx_len);
            SPI_TIMING_END();

#if SPI_DEBUG_PRINT
            debug_printf("\tSent:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\ttx[%d]:%x\n", i, tx_buf[i]);
            }

            debug_printf("\tReceived:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\trx[%d]:%x\n", i, rx_buf[i]);
            }
#endif

            /* Cleanup */
            if(rx_buf != NULL) { vPortFree(rx_buf); }
            if(tx_buf != NULL) { vPortFree(tx_buf); }

            break;

        case 4: /* end */
            test = 0;
            debug_printf("SPI Test Finished\n");
            break;
        }
    }
}


void spi_test_create( UBaseType_t priority )
{

    soc_peripheral_t dev;

    dev = spi_master_driver_init(
            BITSTREAM_SPI_DEVICE_A,  /* Initializing SPI device A */
            0);                      /* This device's interrupts should happen on core 0 */

    xTaskCreate(spi_test, "spi_test", portTASK_STACK_DEPTH(spi_test), dev, priority, NULL);
}
