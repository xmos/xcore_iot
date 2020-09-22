// Copyright (c) 2020, XMOS Ltd, All rights reserved

#define DEBUG_UNIT SPI_CAMERA

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "i2c_driver.h"
#include "spi_master_driver.h"

/* App headers */
#include "spi_camera.h"
#include "ov2640.h"
#include "ai_driver.h"
#include "app_conf.h"

static void camera_task( void *arg )
{
    QueueHandle_t output_queue = ( QueueHandle_t ) arg;
    uint8_t* img_buf = NULL;
    uint8_t tx_buf = ARDUCAM_BURST_FIFO_READ;

    /* Setup and start first capture */
    ov2640_flush_fifo();
    ov2640_clear_fifo_flag();
    ov2640_start_capture();

    while( 1 )
    {
        img_buf = pvPortMalloc( sizeof( uint8_t ) * IMAGE_BUF_SIZE );
        configASSERT( img_buf != NULL );

        /* Poll until capture is completed */
        while( !ov2640_capture_done() )
        {
            vTaskDelay( pdMS_TO_TICKS( 1 ) );
        }

        debug_printf("arducam buffer has %d bytes ready\n", ov2640_read_fifo_length());

        ov2640_spi_read_buf( img_buf, IMAGE_BUF_SIZE, &tx_buf, 1 );

        debug_printf("Arducam -> FreeRTOS done\n");

        if( xQueueSend( output_queue, &img_buf, pdMS_TO_TICKS( 1 ) ) == errQUEUE_FULL )
        {
            debug_printf( "Camera frame lost\n" );
            vPortFree( img_buf );
        }

        ov2640_clear_fifo_flag();
        ov2640_start_capture();
    }
}

static void setup()
{
    ov2640_init( BITSTREAM_SPI_DEVICE_A, BITSTREAM_I2C_DEVICE_A );
    ov2640_configure();
}

void create_spi_camera_to_queue( UBaseType_t priority, QueueHandle_t q_output )
{
    /* Setup camera */
    setup();

    /* Create camera task to take images and feed them to queue */
    xTaskCreate( camera_task, "camera", portTASK_STACK_DEPTH( camera_task ), q_output , priority, NULL );
}
