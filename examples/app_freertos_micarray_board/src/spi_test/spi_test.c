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
#define        tx_len             4
static uint8_t test_msg[tx_len] = { 0xDE, 0xAD, 0xBE, 0xEF };

static void spi_test(void *arg)
{
    soc_peripheral_t spi_dev = arg;
    QueueHandle_t queue = soc_peripheral_app_data(spi_dev);
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

    for(;;)
    {
        vTaskDelay( pdMS_TO_TICKS( test_interval_ms ) );
        switch(test++)
        {
        default:
        case 0:
            debug_printf("SPI Test Start\n");
            break;
        case 1: /* transmit only */
            debug_printf("Transmit Test Start\n");

            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);
            spi_transmit(spi_dev, tx_buf, tx_len );

            debug_printf("\tSent:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\ttx[%d]:%x\n", i, tx_buf[i]);
            }

            debug_printf("Transmit Test Complete\n");
            break;
        case 2: /* receive only */
            debug_printf("Receive Test Start\n");

            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            spi_request(spi_dev, rx_buf, tx_len );
            xQueueReceive(queue, &rx_buf, portMAX_DELAY);

            debug_printf("\tReceived:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\trx[%d]:%x\n", i, rx_buf[i]);
            }

            if(rx_buf != NULL)
            {
                vPortFree(rx_buf);
            }

            debug_printf("Receive Test Complete\n");
            break;
        case 3: /* transmit only blocking */
            debug_printf("Transmit Blocking Test Start\n");

            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);
            spi_transmit(spi_dev, tx_buf, tx_len );

            debug_printf("\tSent:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\ttx[%d]:%x\n", i, tx_buf[i]);
            }

            debug_printf("Transmit Blocking Test Complete\n");
            break;
        case 4: /* receive only blocking */
            debug_printf("Receive Blocking Test Start\n");

            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            spi_request_blocking(spi_dev, rx_buf, tx_len );

            debug_printf("\tReceived:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\trx[%d]:%x\n", i, rx_buf[i]);
            }

            if(rx_buf != NULL)
            {
                vPortFree(rx_buf);
            }

            debug_printf("Receive Blocking Test Complete\n");
            break;
        case 5: /* transaction */
            debug_printf("Transaction Test Start\n");

            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);

            spi_transaction(spi_dev, rx_buf, tx_buf, tx_len );

            debug_printf("\tSent:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\ttx[%d]:%x\n", i, tx_buf[i]);
            }

            xQueueReceive(queue, &rx_buf, portMAX_DELAY);

            debug_printf("\tReceived:\n");
            for(int i=0; i<tx_len; i++)
            {
                debug_printf("\trx[%d]:%x\n", i, rx_buf[i]);
            }

            if(rx_buf != NULL)
            {
                vPortFree(rx_buf);
            }

            debug_printf("Transaction Test Complete\n");
            break;
        case 6: /* transaction blocking */
            debug_printf("Transaction Blocking Test Start\n");

            tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );
            rx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*tx_len );

            memcpy(tx_buf, test_msg , sizeof(uint8_t)*tx_len);

            spi_transaction_blocking(spi_dev, rx_buf, tx_buf, tx_len );

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

            if(rx_buf != NULL)
            {
                vPortFree(rx_buf);
            }

            debug_printf("Transaction Blocking Test Complete\n");
            break;
        case 7: /* end */
            test = 0;
            debug_printf("SPI Test Finished\n");
            break;
        }
    }
}



void spi_test_create( UBaseType_t priority )
{
    QueueHandle_t queue;

    soc_peripheral_t dev;

    queue = xQueueCreate(1, sizeof(void *));

    dev = spi_master_driver_init(
            BITSTREAM_SPI_DEVICE_A,             /* Initializing SPI device A */
            2,                                  /* Give this device 1 RX buffer descriptors */
            0,                                  /* Make each DMA RX buffer */
            2,                                  /* Give this device 1 TX buffer descriptors */
            queue,                              /* Queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) spi_master_ISR);   /* The ISR to handle this device's interrupts */

    xTaskCreate(spi_test, "spi_test", portTASK_STACK_DEPTH(spi_test), dev, priority, NULL);
}
