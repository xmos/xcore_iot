// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>

#include "soc.h"
#include "xassert.h"

#include "micarray_dev.h"
#include "mic_array.h"

#include "debug_print.h"

#define PDM_CLOCK_FREQUENCY         (MICARRAYCONF_MASTER_CLOCK_FREQUENCY/(MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER))
#define DECIMATION_FACTOR           (PDM_CLOCK_FREQUENCY / MICARRAYCONF_PDM_INTEGRATION_FACTOR / MICARRAYCONF_SAMPLE_RATE)

static struct {
    /* Data memory for the lib_mic_array decimation FIRs */
    int data[4][THIRD_STAGE_COEFS_PER_STAGE*DECIMATION_FACTOR];
    mic_array_frame_time_domain comp[MICARRAYCONF_NUM_FRAME_BUFFERS];
    mic_array_frame_time_domain * unsafe current;
    unsigned buffer;
    //q8_24 input_gain;
    mic_array_decimator_conf_common_t dcc;
    mic_array_decimator_config_t dc[MICARRAYCONF_DECIMATOR_COUNT];
} mic_array_data;

// Configures and initializes lib_mic_array.
// Once this returns, the real-time constraint will apply
static unsafe void mic_array_init(
        streaming chanend c_ds_output[MICARRAYCONF_DECIMATOR_COUNT])
{
    const int fir_gain_compen[7] = {
            0,
            FIR_COMPENSATOR_DIV_2,
            FIR_COMPENSATOR_DIV_4,
            FIR_COMPENSATOR_DIV_6,
            FIR_COMPENSATOR_DIV_8,
            0,
            FIR_COMPENSATOR_DIV_12
    };


    const int * unsafe fir_coefs[7] = {
            0,
            g_third_stage_div_2_fir,
            g_third_stage_div_4_fir,
            g_third_stage_div_6_fir,
            g_third_stage_div_8_fir,
            0,
            g_third_stage_div_12_fir
    };

    memset(mic_array_data.data, 0, sizeof(mic_array_data.data));
    memset(mic_array_data.comp, 0, sizeof(mic_array_data.comp));

    //Configure the decimator
    mic_array_data.dcc.len = MIC_ARRAY_MAX_FRAME_SIZE_LOG2;
    mic_array_data.dcc.apply_dc_offset_removal = 1;
    mic_array_data.dcc.index_bit_reversal = 0;
    mic_array_data.dcc.windowing_function = NULL;
    mic_array_data.dcc.output_decimation_factor = DECIMATION_FACTOR;
    mic_array_data.dcc.coefs = fir_coefs[DECIMATION_FACTOR/2];;
    mic_array_data.dcc.apply_mic_gain_compensation = 1;
    mic_array_data.dcc.fir_gain_compensation = fir_gain_compen[DECIMATION_FACTOR/2];;
    mic_array_data.dcc.buffering_type = DECIMATOR_NO_FRAME_OVERLAP;
    mic_array_data.dcc.number_of_frame_buffers = MICARRAYCONF_NUM_FRAME_BUFFERS;

    xassert(MICARRAYCONF_DECIMATOR_COUNT == 1); //Needs to be modified to support additional decimators
    mic_array_data.dc[0].dcc = &mic_array_data.dcc;
    mic_array_data.dc[0].data = mic_array_data.data[0];
    for (int i = 0; i < 4; i++) {
        mic_array_data.dc[0].mic_gain_compensation[i] = INT_MAX;
    }
    mic_array_data.dc[0].channel_count = MIC_ARRAY_NUM_MICS;

    mic_array_decimator_configure(c_ds_output, MICARRAYCONF_DECIMATOR_COUNT, mic_array_data.dc);

    delay_ticks(150000);
    //Once this is called, the real time constraint applies.
    //  mic_array_get_next_time_domain_frame(...) will need to be called once every
    //  MA_FRAME_SIZE sample times.
    mic_array_init_time_domain_frame(
            c_ds_output,
            MICARRAYCONF_DECIMATOR_COUNT,
            mic_array_data.buffer,
            mic_array_data.comp,
            mic_array_data.dc);
}

#pragma select handler
static void mic_array_get_next_time_domain_frame_sh(
        streaming chanend c_from_decimator0,
        streaming chanend c_from_decimators[])
{
    mic_array_data.current = mic_array_get_next_time_domain_frame(
            c_from_decimators,
            MICARRAYCONF_DECIMATOR_COUNT,
            mic_array_data.buffer,
            mic_array_data.comp,
            mic_array_data.dc);
}

[[combinable]]
void micarray_dev_to_dma(
        soc_peripheral_t peripheral,
        chanend ?data_to_dma_c,
        streaming chanend c_ds_output[])
{
    unsafe {
        mic_array_init(c_ds_output);
    }
    while(1) {
		select {
			case mic_array_get_next_time_domain_frame_sh(c_ds_output[0], c_ds_output):
				unsafe {
					if (!isnull(data_to_dma_c)) {
						soc_peripheral_tx_dma_xfer(
								data_to_dma_c,
								mic_array_data.current->data[0],
								sizeof(int32_t) * (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2));
					} else if (peripheral != NULL) {
						soc_peripheral_tx_dma_direct_xfer(
								peripheral,
								mic_array_data.current->data[0],
								sizeof(int32_t) * (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2));
					}
				}
				break;
		}
	}
}

void mic_array_setup_sdr(
		clock pdmclk,
		in port p_mclk,
		out port p_pdm_clk,
		buffered in port:32 p_pdm_mics,
		int divide)
{
	configure_clock_src_divide(pdmclk, p_mclk, divide/2);
	configure_port_clock_output(p_pdm_clk, pdmclk);
	configure_in_port(p_pdm_mics, pdmclk);
	start_clock(pdmclk);
}

void mic_array_setup_ddr(
		clock pdmclk,
		clock pdmclk6,
		in port p_mclk,
		out port p_pdm_clk,
		buffered in port:32 p_pdm_mics,
		int divide)
{
	configure_clock_src_divide(pdmclk, p_mclk, divide/2);
	configure_clock_src_divide(pdmclk6, p_mclk, divide/4);
	configure_port_clock_output(p_pdm_clk, pdmclk);
	configure_in_port(p_pdm_mics, pdmclk6);

	/* start the faster capture clock */
	start_clock(pdmclk6);
	/* wait for a rising edge on the capture clock */
	partin(p_pdm_mics, 4);
	/* start the slower output clock */
	start_clock(pdmclk);
}

void micarray_dev_init(
        clock pdmclk,
        clock ?pdmclk2,
        in port p_mclk,
        out port p_pdm_clk,
        buffered in port:32 p_pdm_mics)
{
	if(isnull(pdmclk2))
	{
		mic_array_setup_sdr(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics, MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER);
	}
	else
	{
		mic_array_setup_ddr(pdmclk, pdmclk2, p_mclk, p_pdm_clk, p_pdm_mics, MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER);
	}
}

void micarray_dev_task(
        in buffered port:32 p_pdm_mics,
        streaming chanend c_ds_output[])
{
    streaming chan c_4x_pdm_mic_0;

    par {
        mic_array_pdm_rx(p_pdm_mics, c_4x_pdm_mic_0, null);
        mic_array_decimate_to_pcm_4ch(c_4x_pdm_mic_0, c_ds_output[0], MIC_ARRAY_NO_INTERNAL_CHANS);
    }
}

void micarray_dev(
        soc_peripheral_t peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        in buffered port:32 p_pdm_mics)
{
    streaming chan c_ds_output[MICARRAYCONF_DECIMATOR_COUNT+1];

    par {
        micarray_dev_task(p_pdm_mics, c_ds_output);
        micarray_dev_to_dma(peripheral, data_to_dma_c, c_ds_output);
    }
}

[[combinable]]
void micarray_dev_1b_to_dma(
        soc_peripheral_t peripheral,
        chanend ?data_to_dma_c,
        streaming chanend c_ds_output[])
{
	int tmp;
	int32_t mic_samples[(1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2)];

	while (1) {
		select {
			case c_ds_output[0] :> tmp:
			unsafe {
				int * unsafe mic_sample_block;
				mic_sample_block = (int*)tmp;
				for(int i=0; i< (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2); i++)
				{
					mic_samples[i] = mic_sample_block[4*i];
				}

				if (!isnull(data_to_dma_c)) {
					soc_peripheral_tx_dma_xfer(
							data_to_dma_c,
							&mic_samples[0],
							sizeof(int32_t) * (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2));
				} else if (peripheral != NULL) {
					soc_peripheral_tx_dma_direct_xfer(
							peripheral,
							&mic_samples[0],
							sizeof(int32_t) * (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2));
				}
			}
			break;
		}
	}
}

void micarray_dev_1b(
        soc_peripheral_t peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        in buffered port:32 p_pdm_mics)
{
    streaming chan c_ds_output[2];
    streaming chan c_4x_pdm_mic_0[1];

    par {
        mic_dual_pdm_rx_decimate(p_pdm_mics, c_ds_output[0], c_4x_pdm_mic_0);
        micarray_dev_1b_to_dma(peripheral, data_to_dma_c, c_ds_output);
    }
}
