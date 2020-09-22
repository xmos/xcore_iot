// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include <timer.h>
#include <string.h>
#include <xcore/parallel.h>
#include <xcore/channel_streaming.h>
#include <xcore/select.h>
#include <xs1.h>
#include <limits.h>

#include "soc.h"
#include "xassert.h"

#include "micarray_dev.h"
//#include "mic_array.h"
#include "fir_coefs.h"
#include "mic_array_frame.h"

#include "debug_print.h"

#define PDM_CLOCK_FREQUENCY         (MICARRAYCONF_MASTER_CLOCK_FREQUENCY/(MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER))
#define DECIMATION_FACTOR           (PDM_CLOCK_FREQUENCY / MICARRAYCONF_PDM_INTEGRATION_FACTOR / MICARRAYCONF_SAMPLE_RATE)


////////// DUPLICATED FROM mic_array.h//////////////////
typedef enum {
    DECIMATOR_NO_FRAME_OVERLAP,   ///<  The frames have no overlap.
    DECIMATOR_HALF_FRAME_OVERLAP  ///<  The frames have a 50% overlap betweeen sequential frames.
} mic_array_decimator_buffering_t;

typedef struct {

    unsigned len; /**< If len is less than 16 then this sets the frame size to 2 to the power of len, i.e. A frame will contain 2 to the power of len samples of each channel.
                                   If len is 16 or greater then the frame size is equal to len. */

    int apply_dc_offset_removal; /**< Remove the DC offset from the audio before the final decimation. Set to non-zero to enable. */

    int index_bit_reversal; /**< If non-zero then bit reverse the index of the elements within the frame. Used in the case of preparing for an FFT.*/

    int * windowing_function; /**< If non-null then this will apply a windowing function to the frame. Used in the case of preparing for an FFT. */

    unsigned output_decimation_factor; /**< Final stage FIR Decimation factor. */

    const int * coefs; /**< The coefficients for the FIR decimator. */

    int apply_mic_gain_compensation; /**< Set to non-zero to apply microphone gain compensation. */

    int fir_gain_compensation; /**< 5.27 format for the gain compensation for the three stages of FIR filter. */

    mic_array_decimator_buffering_t buffering_type;  /**< The buffering type used for frame exchange. */

    unsigned number_of_frame_buffers;  /**< The count of frames used between the decimators and the application. */

} mic_array_decimator_conf_common_t;

/** Configuration structure unique to each of the 4 channel deciamtors.
 *
 *  This contains configuration that is channel specific, i.e. Gain compensation, etc.
 */
typedef struct {

    mic_array_decimator_conf_common_t * dcc;

    int * data;    /**< The data for the FIR decimator */

    int mic_gain_compensation[4]; /**< An array describing the relative gain compensation to apply to the microphones. The microphone with the least gain is defined as 0x7fffffff (INT_MAX), all others are given as INT_MAX*min_gain/current_mic_gain.*/

    unsigned channel_count; /**< The count of enabled channels (0->4).  */


    unsigned async_interface_enabled; /** If set to 1, this disables the mic_array_get_next_time_domain_frame interface
                                        and enables the mic_array_recv_sample interface. **/


} mic_array_decimator_config_t;
//////////////////////////////////////////////////


void mic_array_decimator_configure(
        /*streaming*/ chanend_t c_from_decimators[],
        unsigned decimator_count,
        mic_array_decimator_config_t dc[]);

void mic_array_init_time_domain_frame(
		/*streaming*/ chanend_t c_from_decimators[],
		unsigned decimator_count,
        unsigned *buffer,
		mic_array_frame_time_domain audio[],
        mic_array_decimator_config_t dc[]);


static struct {
    /* Data memory for the lib_mic_array decimation FIRs */
    int data[4][THIRD_STAGE_COEFS_PER_STAGE*DECIMATION_FACTOR];
    mic_array_frame_time_domain comp[MICARRAYCONF_NUM_FRAME_BUFFERS];
    mic_array_frame_time_domain * current;
    unsigned buffer;
    //q8_24 input_gain;
    mic_array_decimator_conf_common_t dcc;
    mic_array_decimator_config_t dc[MICARRAYCONF_DECIMATOR_COUNT];
} mic_array_data;

// Configures and initializes lib_mic_array.
// Once this returns, the real-time constraint will apply
static void mic_array_init(
        /*streaming*/ chanend_t c_ds_output[MICARRAYCONF_DECIMATOR_COUNT])
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


    const int * fir_coefs[7] = {
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
            &mic_array_data.buffer,
            mic_array_data.comp,
            mic_array_data.dc);
}

#if 0
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

static void micarray_dev_to_dma(
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
#endif

static void mic_array_setup_sdr(
		xclock_t pdmclk,
		port_t p_mclk,
		port_t p_pdm_clk,
		port_t p_pdm_mics,
		int divide)
{
	clock_set_source_port(pdmclk, p_mclk);
	clock_set_divide(pdmclk, divide/2);
	//configure_clock_src_divide(pdmclk, p_mclk, divide/2);

	port_set_clock(p_pdm_clk, pdmclk);
	port_set_out_clock(p_pdm_clk);
	//configure_port_clock_output(p_pdm_clk, pdmclk);

	port_set_clock(p_pdm_mics, pdmclk);
	//configure_in_port(p_pdm_mics, pdmclk);

	clock_start(pdmclk);
	//start_clock(pdmclk);
}

static void mic_array_setup_ddr(
		xclock_t pdmclk,
		xclock_t pdmclk6,
		port_t p_mclk,
		port_t p_pdm_clk,
		port_t p_pdm_mics,
		int divide)
{
	clock_set_source_port(pdmclk, p_mclk);
	clock_set_divide(pdmclk, divide/2);
	//configure_clock_src_divide(pdmclk, p_mclk, divide/2);

	clock_set_source_port(pdmclk6, p_mclk);
	clock_set_divide(pdmclk6, divide/4);
	//configure_clock_src_divide(pdmclk6, p_mclk, divide/4);

	port_set_clock(p_pdm_clk, pdmclk);
	port_set_out_clock(p_pdm_clk);
	//configure_port_clock_output(p_pdm_clk, pdmclk);

	port_set_clock(p_pdm_mics, pdmclk6);
	//configure_in_port(p_pdm_mics, pdmclk6);

	/* start the faster capture clock */
	//start_clock(pdmclk6);
	clock_start(pdmclk6);
	/* wait for a rising edge on the capture clock */
	//partin(p_pdm_mics, 4);
	asm volatile("setpsc res[%0], %1" : : "r" (p_pdm_mics), "r"(4));
	port_in(p_pdm_mics);

	/* start the slower output clock */
	//start_clock(pdmclk);
	clock_start(pdmclk);
}

void micarray_dev_init(
        xclock_t pdmclk,
		xclock_t pdmclk2,
        port_t p_mclk,
		port_t p_pdm_clk,
		port_t p_pdm_mics)
{
	xassert(pdmclk != 0);

	if (pdmclk2 == 0)
	{
		mic_array_setup_sdr(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics, MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER);
	}
	else
	{
		mic_array_setup_ddr(pdmclk, pdmclk2, p_mclk, p_pdm_clk, p_pdm_mics, MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER);
	}
}

#if 0
static void micarray_dev_task(
        port_t p_pdm_mics,
		/*streaming*/ chanend_t c_ds_output[])
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
#endif

DECLARE_JOB(micarray_dev_1b_to_dma, (soc_peripheral_t, chanend_t, chanend_t));
void micarray_dev_1b_to_dma(
        soc_peripheral_t peripheral,
        chanend_t data_to_dma_c,
        /*streaming*/ chanend_t c_2x_pdm_mic)
{
	int32_t mic_samples[(1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2)];

	SELECT_RES(
		CASE_THEN(c_2x_pdm_mic, on_c_2x_pdm_mic))
	{
		on_c_2x_pdm_mic: {
			int *mic_sample_block;
			mic_sample_block = (int *) s_chan_in_word(c_2x_pdm_mic);

			for(int i=0; i< (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2); i++)
			{
				mic_samples[i] = mic_sample_block[4*i];
			}

			if (data_to_dma_c != 0) {
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

			continue;
		}
	}
}

void mic_dual_pdm_rx_decimate(port_t p_pdm_mic, chanend_t c_2x_pdm_mic, chanend_t *c_ref_audio);
DECLARE_JOB(mic_dual_pdm_rx_decimate, (port_t, chanend_t, chanend_t *));

void micarray_dev_1b(
        soc_peripheral_t peripheral,
        chanend_t data_to_dma_c,
		chanend_t data_from_dma_c,
		chanend_t ctrl_c,
        port_t p_pdm_mics)
{
    streaming_channel_t c_2x_pdm_mic;
    streaming_channel_t c_ref_audio[2];

    (void) data_from_dma_c;
    (void) ctrl_c;

    c_2x_pdm_mic = s_chan_alloc();
    c_ref_audio[0] = s_chan_alloc();
    c_ref_audio[1] = s_chan_alloc();

    chanend_t ref_audio_ends[2] = {c_ref_audio[0].end_a, c_ref_audio[1].end_a};

    PAR_JOBS (
        PJOB(mic_dual_pdm_rx_decimate, (p_pdm_mics, c_2x_pdm_mic.end_a, ref_audio_ends)),
        PJOB(micarray_dev_1b_to_dma, (peripheral, data_to_dma_c, c_2x_pdm_mic.end_b))
    );
}
