// Copyright (c) 2020, XMOS Ltd, All rights reserved

#define DEBUG_UNIT AI_DEV

#include <xcore/triggerable.h>
#include <xcore/chanend.h>
#include <xmos_flash.h>
#include <timer.h>
#include <string.h>

#include "xassert.h"

#include "rtos_support.h"
#include "soc.h"

#include "ai_dev.h"

#if 0
__attribute__((weak))
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
#ifndef TESTING_DISABLE_SWMEM
    if (ctrl_swmem_c != 0) {
    	TRIGGERABLE_SETUP_EVENT_VECTOR(ctrl_swmem_c, swmem_event_ctrl);
    	triggerable_enable_trigger(ctrl_swmem_c);
    }
#endif
    for (;;) {
        if ( ( setup == 1 ) && ( run_engine == 1 ) )
        {
            debug_printf("AI_Device Running inference...\n");
            TfLiteStatus invoke_status = interpreter->Invoke();
            debug_printf("AI_Device inference done\n");
            run_engine = 0;

            if (invoke_status != kTfLiteOk) {
                debug_printf("Invoke failed/n");
            }
            int output_tensor_size = output->bytes;

            /* DMA transfer resulting tensor */
            xassert( output_tensor_size <= AI_OUTPUT_BYTES_LEN);
            soc_peripheral_tx_dma_xfer(data_to_dma_c, output->data.int8, output_tensor_size);
        }

#ifndef TESTING_DISABLE_SWMEM
		TRIGGERABLE_WAIT_EVENT(event_ctrl, swmem_event_ctrl);

		swmem_event_ctrl: {
            soc_peripheral_function_code_rx(ctrl_swmem_c, &cmd);
			switch( cmd ) {
			case QSPI_DEV_SWMEM_SETUP: /* Setup */
				debug_printf( "SWMem setup\n" );
                swmem_fill_handle = swmem_fill_get();
                soc_peripheral_varlist_tx(ctrl_swmem_c, 1,
                                          sizeof(swmem_fill_t), &swmem_fill_handle);

                run_async(swmem_handler, NULL, stack_base(swmem_handler_stack, 16));

  				debug_printf( "SWMem setup done\n" );
				break;

			case QSPI_DEV_SWMEM_READ: /* Read */
				debug_printf( "SWMem read\n" );
                uintptr_t adr;
                size_t size;
                qspi_flash_dev_cmd_t flash_cmd;
                recv_buf_t recv_buf;

                soc_peripheral_varlist_rx(ctrl_swmem_c, 3,
                                          sizeof(swmem_fill_t), &swmem_fill_handle,
                                          sizeof(uintptr_t), &adr,
                                          sizeof(size_t), &size);

                flash_cmd.operation = qspi_flash_dev_op_read;
                flash_cmd.byte_address = adr;
                flash_cmd.byte_count = size;

                recv_buf.cmd = flash_cmd;
                recv_buf.byte_buf;

      			return_buf = handle_op_request(&flash_handle, &recv_buf, sizeof(recv_buf.cmd));
                soc_peripheral_varlist_tx(ctrl_swmem_c, 1,
                                          size, return_buf);
				break;

            case QSPI_DEV_SWMEM_FILL: /* Fill */
				debug_printf( "SWMem break\n" );
                break;

			case QSPI_DEV_SWMEM_WRITE: /* Write */
				debug_printf( "SWMem write\n" );
				break;

			case QSPI_DEV_SWMEM_DESTROY: /* Destroy */
				debug_printf( "SWMem destroy\n" );
				break;

			default:
				debug_printf( "Invalid SWMEM CMD: %d\n", cmd );
				break;
			}
			continue;
		}
#endif
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
            case TF_DEV_SETUP:
                debug_printf("TF_Device setup tflite\n");
#ifndef TESTING_DISABLE_SWMEM
#if defined(USE_SWMEM) || defined(USE_EXTMEM)
                    // start SW_Mem handler
                    swmem_setup( ctrl_swmem_c );
                    // This can only be done when qspi is on same core
                    //run_async(swmem_handler, NULL, stack_base(swmem_handler_stack, 16));
#endif /* defined(USE_SWMEM) || defined(USE_EXTMEM) */
#endif /* TESTING_DISABLE_SWMEM */
                setup_tflite();
                setup = 1;

                num_bytes = output->bytes;
                soc_peripheral_varlist_tx(ctrl_c, 1,
                                          sizeof( int32_t ), &num_bytes);
                break;
            case TF_DEV_INVOKE:
                /* Call invoke */
                run_engine = 1;
                break;
            case TF_DEV_SET_INPUT_TENSOR:
                num_bytes = 0;
                chunk = 0;
                soc_peripheral_varlist_rx(ctrl_c, 1,
                                          sizeof( int32_t ), &num_bytes);

                xassert( num_bytes <= TF_INPUT_CHUNK_BYTES_LEN );

                soc_peripheral_varlist_rx(ctrl_c, 2,
                                          num_bytes, &buf,
                                          sizeof( int32_t ), &chunk );
                if( setup == 1 )
                {
                    memcpy( input->data.raw + ( chunk * TF_INPUT_CHUNK_BYTES_LEN ), buf, num_bytes );
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
#endif
#if 0
__attribute__((weak))
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
            debug_printf("AI_Device Running inference...\n");
            /* Call for inference */

            /* Transfer output tensor */
            soc_peripheral_tx_dma_xfer(data_to_dma_c, NULL, 0);
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
                debug_printf("AI_Device setup\n");
                setup = 1;

                /* Return output tensor size in bytes */
                num_bytes = 0;
                soc_peripheral_varlist_tx(ctrl_c, 1,
                                          sizeof( int32_t ), &num_bytes);
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

                xassert( num_bytes <= AI_INPUT_CHUNK_BYTES_LEN );

                soc_peripheral_varlist_rx(ctrl_c, 2,
                                          num_bytes, &buf,
                                          sizeof( int32_t ), &chunk );
                if( setup == 1 )
                {
                    /* Copy chunk into input tensor */
                    //memcpy( /* input tensor start */ + ( chunk * AI_INPUT_CHUNK_BYTES_LEN ), buf, num_bytes );
                }
                break;
			default:
				debug_printf( "AI_Device Invalid CTRL CMD\n" );
				break;
			}
			continue;
		}
    }
}
#endif
