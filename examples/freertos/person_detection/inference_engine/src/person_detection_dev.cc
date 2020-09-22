// Copyright (c) 2020, XMOS Ltd, All rights reserved

#define DEBUG_UNIT PERSON_DETECTION_DEV

#ifdef __cplusplus
extern "C" {
#endif

#include <xcore/triggerable.h>
#include <xcore/chanend.h>
#include <xmos_flash.h>
#include <timer.h>
#include <string.h>

#include "FreeRTOS.h"
#include "rtos_support.h"
#include "soc.h"

#include "ai_dev.h"

#include "person_detect_model.h"
#include "inference_engine.h"

#define SHOW_TIMING_DEBUG   0

#if( SHOW_TIMING_DEBUG == 1 )
#include "xscope.h"
static int start_time = 0;
#endif


#define TENSOR_ARENA_SIZE   (1024*128)
uint8_t tensor_arena[TENSOR_ARENA_SIZE];

static size_t input_size;
static uint8_t *input_buffer;

static size_t output_size;
static uint8_t *output_buffer;

/* Override weak ai_dev with person detection */
void ai_dev(
        soc_peripheral_t peripheral,
		chanend data_to_dma_c,
        chanend data_from_dma_c,
		chanend ctrl_c,
		chanend ctrl_swmem_c)
{
    uint32_t cmd;
    uint8_t buf[AI_INPUT_CHUNK_BYTES_LEN];
    int32_t num_bytes = 0;
    int32_t chunk = 0;
    int run_engine = 0;
    int setup = 0;

    triggerable_disable_all();

    if (ctrl_c != 0) {
    	TRIGGERABLE_SETUP_EVENT_VECTOR(ctrl_c, event_ctrl);
    	triggerable_enable_trigger(ctrl_c);
    }

    for (;;) {
        if ( ( setup == 1 ) && ( run_engine == 1 ) )
        {
            debug_printf("TF_Device Running inference...\n");
#if( SHOW_TIMING_DEBUG == 1 )
            start_time = xscope_gettime();
            invoke();
            debug_printf( "Inference Duration:%d us\n", (xscope_gettime() - start_time) / 100 );
#else
            invoke();
#endif
            debug_printf("TF_Device inference done\n");
            run_engine = 0;

            /* DMA transfer resulting tensor */
            configASSERT( output_size <= AI_OUTPUT_BYTES_LEN);
            soc_peripheral_tx_dma_xfer(data_to_dma_c, output_buffer, output_size);
        }

		event_ctrl: {
			soc_peripheral_function_code_rx(ctrl_c, &cmd);
			switch( cmd ) {
			case SOC_PERIPHERAL_DMA_TX:
				/*
				 * The application has added a new DMA TX buffer. This
				 * ensures that this select statement wakes up and gets
				 * the TX data in the code above.
				 */
				break;
            case AI_DEV_SETUP:
                debug_printf("TF_Device setup tflite\n");

                /* Setup Runtime */
                initialize(person_detect, tensor_arena, TENSOR_ARENA_SIZE, &input_buffer,
                           &input_size, &output_buffer, &output_size);

                debug_printf("TF_Device setup tflite done\n");
                setup = 1;

                soc_peripheral_varlist_tx(ctrl_c, 1,
                                          sizeof( int32_t ), &output_size);
                break;
            case AI_DEV_INVOKE:
                /* Call invoke */
                run_engine = 1;
                break;
            case AI_DEV_SET_INPUT_TENSOR:
                num_bytes = 0;
                chunk = 0;
                soc_peripheral_varlist_rx(ctrl_c, 1,
                                          sizeof( int32_t ), &num_bytes);

                configASSERT( num_bytes <= AI_INPUT_CHUNK_BYTES_LEN );

                soc_peripheral_varlist_rx(ctrl_c, 2,
                                          num_bytes, &buf,
                                          sizeof( int32_t ), &chunk );

                // TODO: rx chunk and then rx directly into input buffer to avoid copy
                if( setup == 1 )
                {
                    memcpy( input_buffer + ( chunk * AI_INPUT_CHUNK_BYTES_LEN ), buf, num_bytes );
                }
                break;
			default:
				debug_printf( "TF_Device Invalid CTRL CMD\n" );
				break;
			}
			continue;
		}
    }
}

#ifdef __cplusplus
}
#endif
