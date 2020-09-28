// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */

/* App headers */
#include "soc.h"
#include "bitstream_devices.h"
#include "qspi_flash_driver.h"
#include "ai_driver.h"
#include "fs_support.h"
#include "ff.h"

#define SHOW_TIMING_DEBUG   1

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

#if( SHOW_TIMING_DEBUG == 1 )
#include "xscope.h"

static int start_time = 0;
static int end_time = 0;
#endif

void soc_tile0_main(
        int tile)
{
    vTaskStartScheduler();
}

static int argmax(const int8_t *A, const int N) {
  configASSERT(N > 0);

  int m = 0;

  for (int i = 1; i < N; i++) {
    if (A[i] > A[m]) {
      m = i;
    }
  }

  return m;
}

typedef struct cifar10_task_args {
    QueueHandle_t queue;
} cifar10_task_args_t;

AI_ISR_CALLBACK_FUNCTION( ai_dev_callback, buf, len, status, args, xYieldRequired )
{
    BaseType_t yield = pdFALSE;
    cifar10_task_args_t* task_args = (cifar10_task_args_t*)args;

    if( status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM )
    {
        if( xQueueSendFromISR( task_args->queue, buf, &yield ) == errQUEUE_FULL ) {
            debug_printf("AI ISR to task queue full\n");
        }
    }

    *xYieldRequired = yield;
}

static const char* test_input_files[] = {
	"airplane.bin",
    "bird.bin",
    "cat.bin",
    "deer.bin",
    "frog.bin",
    "horse.bin",
    "truck.bin"
};

static void cifar10_task( void *arg )
{
    cifar10_task_args_t* task_args = pvPortMalloc( sizeof( cifar10_task_args_t ) );
    soc_peripheral_t dev = ai_driver_init( 0, ai_dev_callback, task_args );
    FIL current_file;
    unsigned int file_size;
    uint8_t *data = NULL;
    FRESULT result;
    unsigned int bytes_read = 0;
    int32_t output_tensor_bytes = 0;
    char classification[12] = {0};

    output_tensor_bytes = ai_setup( dev );
    task_args->queue = xQueueCreate( 1, output_tensor_bytes );
    uint8_t* output_tensor_buf = pvPortMalloc( output_tensor_bytes );

	while( 1 )
	{
        for( int i=0; i<( sizeof( test_input_files )/sizeof( test_input_files[0] ) ); i++ )
        {
#if( SHOW_TIMING_DEBUG == 1 )
            start_time = xscope_gettime();
#endif

            get_file(test_input_files[i], &current_file, &file_size );
		    data = pvPortMalloc( sizeof( unsigned char ) * file_size );

            configASSERT( data != NULL); /* Failed to allocate memory for file data */

            result = f_read( &current_file, data, file_size, &bytes_read );

#if( SHOW_TIMING_DEBUG == 1 )
            end_time = xscope_gettime();
            debug_printf("\n****read file from disk time: %d us\n", (end_time - start_time) / 100 );
#endif

            if( bytes_read == file_size )
            {
                debug_printf("FreeRTOS set input tensor to %s\n", test_input_files[i]);

#if( SHOW_TIMING_DEBUG == 1 )
                start_time = xscope_gettime();
#endif
                ai_set_input_tensor(dev, data, file_size);

#if( SHOW_TIMING_DEBUG == 1 )
                end_time = xscope_gettime();
                debug_printf("****setup input tensor time: %d us\n", (end_time - start_time) / 100 );
#endif
                debug_printf("FreeRTOS invoke\n");
#if( SHOW_TIMING_DEBUG == 1 )
                start_time = xscope_gettime();
#endif
                ai_invoke( dev );
            }
            else
            {
                debug_printf("Failed to load %s\n", test_input_files[i]);;
            }

            xQueueReceive( task_args->queue, output_tensor_buf, portMAX_DELAY);

#if( SHOW_TIMING_DEBUG == 1 )
            end_time = xscope_gettime();
            debug_printf("********received result time: %d us\n", (end_time - start_time) / 100 );
#endif

            int m = argmax((const int8_t*)output_tensor_buf, 10);
            switch (m) {
            case 0:
              snprintf(classification, 9, "Airplane");
              break;
            case 1:
              snprintf(classification, 11, "Automobile");
              break;
            case 2:
              snprintf(classification, 5, "Bird");
              break;
            case 3:
              snprintf(classification, 4, "Cat");
              break;
            case 4:
              snprintf(classification, 5, "Deer");
              break;
            case 5:
              snprintf(classification, 4, "Dog");
              break;
            case 6:
              snprintf(classification, 5, "Frog");
              break;
            case 7:
              snprintf(classification, 6, "Horse");
              break;
            case 8:
              snprintf(classification, 5, "Ship");
              break;
            case 9:
              snprintf(classification, 6, "Truck");
              break;
            default:
              break;
            }
            debug_printf("FreeRTOS Classification = %s\n", classification);

            vPortFree( data );
        }
        debug_printf("\nAll Cifar10 examples have been run.\nTerminating task...\n");
        vTaskDelete( NULL );
	}
}

void vApplicationDaemonTaskStartupHook( void )
{
    /* Initialize filesystem  */
	filesystem_init();

    xTaskCreate( cifar10_task, "cifar10", portTASK_STACK_DEPTH( cifar10_task ), NULL , configMAX_PRIORITIES, NULL );
}

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
}
