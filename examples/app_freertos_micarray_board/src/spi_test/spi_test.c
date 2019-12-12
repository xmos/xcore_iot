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

#define appconfSPI_BUFFER       4096
#define appconfSPI_SPEED_KHZ    125
#define appconfSPI_MODE         SPI_MODE_1

static void spi_test(void *arg)
{
    soc_peripheral_t spi_dev = arg;
//    soc_dma_ring_buf_t *tx_ring_buf = soc_peripheral_tx_dma_ring_buf(spi_dev);

//    spi_driver_setup(spi_dev, appconfSPI_SPEED_KHZ, appconfSPI_MODE);

    uint8_t *tx_buf;

    for(;;)
    {
//        debug_printf("sent\n");
        tx_buf = (uint8_t*)pvPortMalloc( sizeof(uint8_t)*4);

        tx_buf[0] = 0xDE;
        tx_buf[1] = 0xAD;
        tx_buf[2] = 0xBE;
        tx_buf[3] = 0xEF;

        spi_driver_setup(spi_dev, appconfSPI_SPEED_KHZ, appconfSPI_MODE);
        spi_driver_transmit(spi_dev, tx_buf, 4 );
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }
}



void spi_test_create( UBaseType_t priority )
{
    soc_peripheral_t dev;

    dev = spi_driver_init(
            BITSTREAM_SPI_DEVICE_A,             /* Initializing SPI device A */
            2,                                  /* Give this device 1 RX buffer descriptors */
            appconfSPI_BUFFER * sizeof(int32_t), /* Make each DMA RX buffer */
            2,                                  /* Give this device 1 TX buffer descriptors */
            NULL,                               /* Queue associated with this device */
            0,                                  /* This device's interrupts should happen on core 0 */
            (rtos_irq_isr_t) spi_ISR);          /* The ISR to handle this device's interrupts */

    xTaskCreate(spi_test, "spi_test", portTASK_STACK_DEPTH(spi_test), dev, priority, NULL);
}
