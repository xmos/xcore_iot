// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#define DEBUG_UNIT SDRAM_TEST
#include "app_conf.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "sdram_driver.h"

/* App headers */
#include "sdram_test.h"
#include "app_conf.h"

void vsdram_test( void *arg )
{
    soc_peripheral_t xSDRAM_dev = arg;

    char test1[12] = "hello world";
    char test2[14] = "goodbye world";
    char readbuf[SDRAMCONF_READ_BUFFER_SIZE];

    int addr1 = 0;
    int addr2 = 56;
    for(;;)
    {
        memset(&readbuf, 0x00, sizeof(readbuf[0]) * SDRAMCONF_READ_BUFFER_SIZE );
        debug_printf("Write %s to %d\n", test1, addr1);
        sdram_driver_write(xSDRAM_dev, addr1, 12, &test1);

        debug_printf("Write %s to %d\n", test2, addr2);
        sdram_driver_write(xSDRAM_dev, addr2, 14, &test2);

        vTaskDelay(pdMS_TO_TICKS(100));

        sdram_driver_read(xSDRAM_dev, addr1, SDRAMCONF_READ_BUFFER_SIZE, &readbuf);
        debug_printf("Read %s at %d\n", readbuf, addr1);

        sdram_driver_read(xSDRAM_dev, addr2, SDRAMCONF_READ_BUFFER_SIZE, &readbuf);
        debug_printf("Read %s at %d\n", readbuf, addr2);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void sdram_test_create( UBaseType_t priority )
{
    soc_peripheral_t dev;

    dev = sdram_driver_init(
            BITSTREAM_SDRAM_DEVICE_A,       /* Initializing SDRAM device A */
            0,                              /* The number of DMA RX buffer descriptors */
            0,                              /* Do not allocate any RX buffers yet */
            0,                              /* The number of DMA TX buffer descriptors */
            NULL,                           /* Nothing associated with this device */
            0,                              /* The core that will be interrupted on DMA events */
            NULL);                          /* No ISR associated with this device */

    xTaskCreate( vsdram_test, "sdram_test", portTASK_STACK_DEPTH(vsdram_test), dev, priority, NULL );
}
