// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_driver.h"

/* App headers */
#include "spi_test.h"
#include "debug_print.h"

#define appconfSPI_BUFFER       4096
#define appconfSPI_SPEED_KHZ    125
#define appconfSPI_MODE         SPI_MODE_3


#define appconfSPI_CS_PORT_BIT  0
#define appconfSPI_CPOL         0
#define appconfSPI_CPHA         0

/* 100 MHz / (2 * 7) / 2 = 3.57 MHz SCK. aardvark slave max br is 4MHz */
#define appconfSPI_CLOCKDIV     7

/* tsu_cs on WF200 is 3 ns. aardvark is 10000 (10 us) */
#define appconfSPI_CS_TO_DATA_DELAY_NS  10000

/* td on WF200 is 0 ns. aardvark is 4000 (4 us) */
#define appconfSPI_BYTE_SETUP_NS    4000


static void spi_test(void *arg)
{
    soc_peripheral_t spi_dev = arg;
    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(spi_dev);

    QueueHandle_t queue = soc_peripheral_app_data(spi_dev);

    spi_device_init(spi_dev,
                    appconfSPI_CS_PORT_BIT,
                    appconfSPI_CPOL,
                    appconfSPI_CPHA,
                    appconfSPI_CLOCKDIV,
                    appconfSPI_CS_TO_DATA_DELAY_NS,
                    appconfSPI_BYTE_SETUP_NS);

    uint8_t *tx_buf;
    uint8_t *rx_buf;

    for(;;)
    {
        debug_printf("sent\n");
        tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*4 );

//        rx_buf = soc_dma_ring_rx_buf_get(rx_ring_buf, &len);
        tx_buf[0] = 0xDE;
        tx_buf[1] = 0xAD;
        tx_buf[2] = 0xBE;
        tx_buf[3] = 0xEF;

//        spi_transmit(spi_dev, tx_buf, 4 );
        spi_transaction(spi_dev, tx_buf, tx_buf, 4);
//        spi_transaction_blocking(spi_dev, tx_buf, rx_buf, 4);

        xQueueReceive(queue, &rx_buf, portMAX_DELAY);
        debug_printf("task rx: %d %d %d %d\n", rx_buf[0],rx_buf[1], rx_buf[2], rx_buf[3]);

//        vPortFree(rx_buf);

        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }
}



void spi_test_create( UBaseType_t priority )
{
    QueueHandle_t queue;

    soc_peripheral_t dev;

    queue = xQueueCreate(1, sizeof(void *));

    dev = spi_driver_init(
            BITSTREAM_SPI_DEVICE_A,             /* Initializing SPI device A */
            2,                                  /* Give this device 1 RX buffer descriptors */
            appconfSPI_BUFFER * sizeof(uint8_t), /* Make each DMA RX buffer */
            2,                                  /* Give this device 1 TX buffer descriptors */
            queue,                               /* Queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) spi_ISR);          /* The ISR to handle this device's interrupts */

    xTaskCreate(spi_test, "spi_test", portTASK_STACK_DEPTH(spi_test), dev, priority, NULL);
}
