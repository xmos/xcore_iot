// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "ai_driver.h"
#include "gpio_driver.h"

/* App headers */
#include "queue_to_ai.h"
#include "spi_camera.h"
#include "app_conf.h"
#include "xscope.h"

#define SHOW_TIMING_DEBUG   0

#if( SHOW_TIMING_DEBUG == 1 )
static int start_time = 0;
#endif

#define INPUT_TENSOR_BUF_SIZE   ( 96*96 )

typedef struct person_detection_task_args {
    QueueHandle_t q_result;
} person_detection_task_args_t;

AI_ISR_CALLBACK_FUNCTION( ai_dev_callback, buf, len, status, args, xYieldRequired )
{
    BaseType_t yield = pdFALSE;
    person_detection_task_args_t* task_args = (person_detection_task_args_t*)args;

    if( status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM )
    {
        if( xQueueSendFromISR( task_args->q_result, buf, &yield ) == errQUEUE_FULL ) {
            debug_printf("AI ISR to task queue full\n");
        }
    }

    *xYieldRequired = yield;
}

void queue_to_ai(void *arg)
{
    QueueHandle_t input_queue = ( QueueHandle_t ) arg;

    soc_peripheral_t ai_dev = NULL;
    soc_peripheral_t gpio_dev = NULL;
    uint8_t* output_tensor_buf = NULL;
    int32_t output_tensor_bytes = 0;
    person_detection_task_args_t* task_args = NULL;
    uint8_t* img_buf = NULL;
    int toggle = 0;

#ifdef OUTPUT_IMAGE_STREAM
    xscope_mode_lossless();
#endif

    task_args = pvPortMalloc( sizeof( person_detection_task_args_t ) );
    configASSERT( task_args != NULL );

    ai_dev = ai_driver_init( 0, ai_dev_callback, task_args );

    output_tensor_bytes = ai_setup( ai_dev );
    task_args->q_result = xQueueCreate( 1, output_tensor_bytes );
    output_tensor_buf = pvPortMalloc( output_tensor_bytes );
    configASSERT( output_tensor_buf != NULL );

    gpio_dev = gpio_driver_init( BITSTREAM_GPIO_DEVICE_A, 0);

    /* Initialize LED outputs */
    gpio_init( gpio_dev, gpio_4C );
    gpio_write(gpio_dev, gpio_4C, 0x00);

    uint8_t ai_img_buf[ INPUT_TENSOR_BUF_SIZE ];
    while( 1 )
    {
#if( SHOW_TIMING_DEBUG == 1 )
        start_time = xscope_gettime();
#endif
        xQueueReceive( input_queue, &img_buf, portMAX_DELAY );

#if( SHOW_TIMING_DEBUG == 1 )
        debug_printf( "Image rx:%d us\n", (xscope_gettime() - start_time) / 100 );
#endif
        /* tmpbuf[i%2] contains the values we want to pass to the ai task */
        for( int i=0; i<IMAGE_BUF_SIZE; i++)
        {
            if( (i%2) )
            {
                ai_img_buf[i>>1] = img_buf[i];
            }
        }
        vPortFree( img_buf );

#if( SHOW_TIMING_DEBUG == 1 )
        start_time = xscope_gettime();
#endif
#ifdef OUTPUT_IMAGE_STREAM
        taskENTER_CRITICAL();
        {
            xscope_bytes( INPUT_IMAGE,
                          INPUT_TENSOR_BUF_SIZE,
                          (const unsigned char*)&ai_img_buf );
        }
        taskEXIT_CRITICAL();
#endif

#if( SHOW_TIMING_DEBUG == 1 )
        debug_printf( "XScope transfer:%d us\n", (xscope_gettime() - start_time) / 100 );

        start_time = xscope_gettime();
#endif
        ai_set_input_tensor( ai_dev, (uint8_t*)&ai_img_buf, INPUT_TENSOR_BUF_SIZE );

#if( SHOW_TIMING_DEBUG == 1 )
        debug_printf( "Set AI input tensor:%d us\n", (xscope_gettime() - start_time) / 100 );

        start_time = xscope_gettime();
#endif
        ai_invoke( ai_dev );

        xQueueReceive( task_args->q_result, output_tensor_buf, portMAX_DELAY );

#if( SHOW_TIMING_DEBUG == 1 )
        debug_printf( "Invoke and get output tensor:%d us\n", (xscope_gettime() - start_time) / 100 );
#endif
        debug_printf( "\nPerson score: %d\nNo person score: %d\n", output_tensor_buf[0], output_tensor_buf[1] );

#ifdef OUTPUT_IMAGE_STREAM
        taskENTER_CRITICAL();
        {
            xscope_bytes( OUTPUT_TENSOR,
                          output_tensor_bytes,
                          (const unsigned char*)output_tensor_buf );
        }
        taskEXIT_CRITICAL();
#endif

        gpio_write_pin( gpio_dev, gpio_4C, 0, output_tensor_buf[0] > output_tensor_buf[1] );
        gpio_write_pin( gpio_dev, gpio_4C, 3, ( toggle++ ) & 0x01 );
    }
}

void create_queue_to_ai( UBaseType_t priority, QueueHandle_t q_input )
{
    xTaskCreate(queue_to_ai, "q2ai", portTASK_STACK_DEPTH(queue_to_ai), q_input, priority, NULL);
}
