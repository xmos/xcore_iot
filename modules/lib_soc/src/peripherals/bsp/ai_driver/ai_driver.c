// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "ai_driver.h"
#include "ai_dev_ctrl.h"
#include "soc_bsp_common.h"

#include "bitstream_devices.h"

#include "FreeRTOS.h"
#include "semphr.h"


#if ( SOC_AI_PERIPHERAL_USED == 0 )
#define BITSTREAM_AI_DEVICE_COUNT 0
soc_peripheral_t bitstream_ai_devices[ BITSTREAM_AI_DEVICE_COUNT ];
#endif /* SOC_AI_PERIPHERAL_USED */

typedef struct {
    soc_peripheral_t dev;
    SemaphoreHandle_t lock;
    AI_ISR_CALLBACK_ATTR ai_isr_cb_t cb;
    void* args;
} ai_driver_t;

RTOS_IRQ_ISR_ATTR
static void ai_isr( soc_peripheral_t dev )
{
    ai_driver_t *driver_struct = ( ai_driver_t * ) soc_peripheral_app_data( dev );
    BaseType_t xYieldRequired = pdFALSE;
    uint32_t status;

    status = soc_peripheral_interrupt_status(dev);

    if( status & SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM )
    {
        soc_dma_ring_buf_t *ring_buf;
        uint8_t *rx_buf;
        int length;

        configASSERT( dev == bitstream_ai_devices[ BITSTREAM_AI_DEVICE_A ] );

        ring_buf = soc_peripheral_rx_dma_ring_buf( dev );
        rx_buf = soc_dma_ring_rx_buf_get( ring_buf, &length, NULL );

        configASSERT( rx_buf != NULL ); /* Valid buffer was expected */

        if( rx_buf != NULL)
        {
            if( driver_struct->cb != NULL )
            {
                driver_struct->cb(rx_buf, length, SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM, driver_struct->args, &xYieldRequired);
            }
            else
            {
                debug_printf("No callback registered, data lost\n");
            }
            soc_dma_ring_rx_buf_set( ring_buf, rx_buf, AI_OUTPUT_BYTES_LEN );
            soc_peripheral_hub_dma_request( dev, SOC_DMA_RX_REQUEST );
        }
    }

    portEND_SWITCHING_ISR(xYieldRequired);
}

int32_t ai_setup( soc_peripheral_t dev )
{
    int32_t retval;
    chanend c_ctrl = soc_peripheral_ctrl_chanend( dev );
    ai_driver_t *driver_struct = ( ai_driver_t * ) soc_peripheral_app_data( dev );

    xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

    soc_peripheral_function_code_tx(c_ctrl, AI_DEV_SETUP);

    soc_peripheral_varlist_rx( c_ctrl, 1,
                               sizeof( int32_t ), &retval );

    xSemaphoreGive(driver_struct->lock);
    return retval;
}

void ai_invoke( soc_peripheral_t dev )
{
    chanend c_ctrl = soc_peripheral_ctrl_chanend( dev );
    ai_driver_t *driver_struct = ( ai_driver_t * ) soc_peripheral_app_data( dev );

    xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

    soc_peripheral_function_code_tx(c_ctrl, AI_DEV_INVOKE);

    xSemaphoreGive(driver_struct->lock);
}

int32_t ai_set_input_tensor( soc_peripheral_t dev, uint8_t* buf, int32_t num_bytes )
{
    int32_t bytes_written = 0;
    int32_t chunk = 0;
    chanend c_ctrl = soc_peripheral_ctrl_chanend( dev );
    ai_driver_t *driver_struct = ( ai_driver_t * ) soc_peripheral_app_data( dev );

    xSemaphoreTake(driver_struct->lock, portMAX_DELAY);

    if( num_bytes <= AI_INPUT_CHUNK_BYTES_LEN )
    {
        soc_peripheral_function_code_tx( c_ctrl, AI_DEV_SET_INPUT_TENSOR );

        soc_peripheral_varlist_tx( c_ctrl, 1,
                                   sizeof( int32_t ), &num_bytes );

        soc_peripheral_varlist_tx( c_ctrl, 2,
                                   num_bytes, buf,
                                   sizeof( int32_t ), &chunk );
        bytes_written = num_bytes;
    }
    else /* Send buffer in chunks of AI_INPUT_CHUNK_BYTES_LEN */
    {
        int32_t bytes_to_write = 0;
        while( bytes_written < num_bytes )
        {
            bytes_to_write = ( ( num_bytes - bytes_written ) > AI_INPUT_CHUNK_BYTES_LEN )
                                ? ( AI_INPUT_CHUNK_BYTES_LEN )
                                : ( num_bytes - bytes_written );
            soc_peripheral_function_code_tx( c_ctrl, AI_DEV_SET_INPUT_TENSOR );

            soc_peripheral_varlist_tx( c_ctrl, 1,
                                       sizeof( int32_t ), &bytes_to_write );

            soc_peripheral_varlist_tx( c_ctrl, 2,
                                       bytes_to_write, buf + ( chunk * AI_INPUT_CHUNK_BYTES_LEN ),
                                       sizeof( int32_t ), &chunk );
            chunk++;
            bytes_written += bytes_to_write;
        }
    }

    xSemaphoreGive(driver_struct->lock);

    return bytes_written;
}

soc_peripheral_t ai_driver_init(
        int isr_core,
		ai_isr_cb_t isr_cb,
        void *args)
{
    soc_peripheral_t device = NULL;
    ai_driver_t *driver_struct;

    driver_struct = pvPortMalloc( sizeof( ai_driver_t ) );

    driver_struct->lock = xSemaphoreCreateMutex();
    driver_struct->dev = device;
    driver_struct->cb = isr_cb;
    driver_struct->args = args;

    device = bitstream_ai_devices[ BITSTREAM_AI_DEVICE_A ];

    soc_peripheral_common_dma_init(
            device,
            1,                             /* Initialize with 1 RX buffer descriptor */
            AI_OUTPUT_BYTES_LEN,           /* Size of each DMA buffer */
            0,                             /* No TX buffers */
			driver_struct,
            isr_core,
            (rtos_irq_isr_t) ai_isr );

    return device;
}
