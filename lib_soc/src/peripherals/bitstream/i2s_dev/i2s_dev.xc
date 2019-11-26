// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>

#include "soc.h"

#include "i2s.h"
#include "i2s_dev.h"

#include "fifo.h"

static int32_t audio_samples[I2SCONF_FRAME_BUF_CNT][I2SCONF_AUDIO_FRAME_LEN];

[[distributable]]
static void i2s_handler(
        server i2s_frame_callback_if i2s,
        fifo_t sample_buffer)
{
    int buf_num;
    int sample_num = 0;

    while (1) {
        select {

        case i2s.init(i2s_config_t &?i2s_config, tdm_config_t &?tdm_config):
            i2s_config.mode = I2S_MODE_I2S;
            i2s_config.mclk_bclk_ratio = I2SCONF_MASTER_CLK_FREQ / (I2SCONF_SAMPLE_FREQ * 32 * 2);

            fifo_get_blocking(sample_buffer, &buf_num);

            break;

        case i2s.send(size_t num_chan_out, int32_t sample[num_chan_out]):
            for (int i = 0; i < num_chan_out; i++) {
                sample[i] = audio_samples[buf_num][sample_num];
            }

            sample_num++;

            if (sample_num == I2SCONF_AUDIO_FRAME_LEN) {
                fifo_get_blocking(sample_buffer, &buf_num);
                sample_num = 0;
            }
            break;

        case i2s.receive(size_t num_chan_in, int32_t sample[num_chan_in]):
            break;

        case i2s.restart_check() -> i2s_restart_t restart:
            restart = I2S_NO_RESTART;
            break;
        }
    }
}

/**
 * This task receives data from the DMA engine into a FIFO.
 * The i2s handler above pulls frames out of the FIFO as
 * needed. This should ensure there is always a next frame
 * already available at the cost of some latency and an
 * extra core.
 */
static void i2s_decoupler(
        chanend c,
        fifo_t sample_buffer)
{
    int buf_num = 0;

    for (;;) {
        uint32_t recv_len;
        xcore_freertos_dma_device_rx_ready(c);
        recv_len = xcore_freertos_dma_device_rx_data(
                c,
                audio_samples[buf_num],
                sizeof(audio_samples[0]));

        fifo_put_blocking(sample_buffer, &buf_num);
        if (++buf_num == I2SCONF_FRAME_BUF_CNT) {
            buf_num = 0;
        }
    }
}

void i2s_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        in port p_mclk,
        out buffered port:32 p_lrclk,
        out port p_bclk,
        out buffered port:32 (&?p_dout)[num_out],
        static const size_t num_out,
        clock bclk)
{
    interface i2s_frame_callback_if i_i2s;
    fifo_t sample_buffer;

    unsafe {
        fifo_init(
                sample_buffer,
                I2SCONF_FRAME_BUF_CNT-1,
                sizeof(int),
                I2SCONF_FRAME_BUF_CNT-1);
    }

    par {
        i2s_frame_master(
                i_i2s,
                p_dout, num_out,
                null, 0,
                p_bclk, p_lrclk,
                p_mclk,
                bclk);


        [[distribute]] i2s_handler(i_i2s, sample_buffer);
        i2s_decoupler(data_from_dma_c, sample_buffer);
    }
}
