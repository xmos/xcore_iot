// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xclib.h>
#include <xcore/port.h>
#include <xcore/clock.h>
#include <xcore/assert.h>

#include "i2s.h"

static void i2s_slave_init_ports(
        /*out buffered*/port_t /*:32*/p_dout[],
        size_t num_out,
        /*in buffered*/port_t /*:32*/p_din[],
        size_t num_in,
        /*in*/port_t p_bclk,
        /*in buffered*/port_t /*:32*/p_lrclk,
        xclock_t bclk)
{
    size_t i;

    clock_enable(bclk);
    port_reset(p_bclk);
    clock_set_source_port(bclk, p_bclk);

    port_start_buffered(p_lrclk, 32);
    port_set_clock(p_lrclk, bclk);

    for (i = 0; i < num_out; i++) {
        port_start_buffered(p_dout[i], 32);
        port_set_clock(p_dout[i], bclk);
        port_out(p_dout[i], 0);
    }

    for (i = 0; i < num_in; i++) {
        port_start_buffered(p_din[i], 32);
        port_set_clock(p_din[i], bclk);
    }

    clock_start(bclk);
}

void i2s_slave(
        const i2s_callback_group_t *const i2s_cbg,
        /*out buffered*/port_t /*:32*/p_dout[],
        const size_t num_out,
        /*in buffered*/port_t /*:32*/p_din[],
        const size_t num_in,
        /*in*/port_t p_bclk,
        /*in buffered*/port_t /*:32*/p_lrclk,
        xclock_t bclk)
{

    port_timestamp_t port_time;
    size_t i;
    size_t idx;

    /* two samples per data line, left and right */
    int32_t in_samps[I2S_CHANS_PER_FRAME * I2S_MAX_DATALINES];
    int32_t out_samps[I2S_CHANS_PER_FRAME * I2S_MAX_DATALINES];

    xassert(num_in <= I2S_MAX_DATALINES);
    xassert(num_out <= I2S_MAX_DATALINES);

    for (;;) {
        i2s_slave_init_ports(p_dout, num_out, p_din, num_in, p_bclk, p_lrclk, bclk);

        i2s_config_t config;
        i2s_restart_t restart = I2S_NO_RESTART;
        i2s_cbg->init(i2s_cbg->app_data, &config);

        //Get initial send data if output enabled
        if (num_out > 0) {
            i2s_cbg->send(i2s_cbg->app_data, num_out << 1, out_samps);
        }

        unsigned mode = config.mode;

        if (config.slave_bclk_polarity == I2S_SLAVE_SAMPLE_ON_BCLK_FALLING) {
            port_set_invert(p_bclk);
        } else {
            port_set_no_invert(p_bclk);
        }

        const unsigned expected_low  = (mode == I2S_MODE_I2S ? 0x80000000 : 0x00000000);
        const unsigned expected_high = (mode == I2S_MODE_I2S ? 0x7fffffff : 0xffffffff);

        unsigned syncerror = 0;
        unsigned lrval;

        for (i = 0; i < num_out; i++) {
            port_clear_buffer(p_dout[i]);
        }
        for (i = 0; i < num_in; i++) {
            port_clear_buffer(p_din[i]);
        }
        port_clear_buffer(p_lrclk);

        unsigned offset = 0;
        if (mode == I2S_MODE_I2S) {
            offset = 1;
        }

        // Wait for LRCLK edge (in I2S LRCLK = 0 is left, TDM rising edge is start of frame)
        port_set_trigger_in_equal(p_lrclk, 1);
        (void) port_in(p_lrclk);
        port_set_trigger_in_equal(p_lrclk, 0);
        (void) port_in(p_lrclk);
        port_time = port_get_trigger_time(p_lrclk);

        unsigned initial_out_port_time = port_time + offset + (I2S_CHANS_PER_FRAME * 32);
        unsigned initial_in_port_time  = port_time + offset + ((I2S_CHANS_PER_FRAME * 32) + 32) - 1;

        //Start outputting evens (0,2,4..) data at correct point relative to the clock
        for (i = 0, idx = 0; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
            port_set_trigger_time(p_dout[i], initial_out_port_time);
            port_out(p_dout[i], bitrev(out_samps[idx]));
        }

        port_set_trigger_time(p_lrclk, initial_in_port_time);
        for (i = 0; i < num_in; i++) {
            port_set_trigger_time(p_din[i], initial_in_port_time);
        }

        //And pre-load the odds (1,3,5..) to follow immediately afterwards
        for (i = 0, idx = 1; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
            port_out(p_dout[i], bitrev(out_samps[idx]));
        }

        //Main loop
        while (!syncerror && (restart == I2S_NO_RESTART)) {
            restart = i2s_cbg->restart_check(i2s_cbg->app_data);

            if (num_out > 0 && (restart == I2S_NO_RESTART)) {
                i2s_cbg->send(i2s_cbg->app_data, num_out << 1, out_samps);

                //Output i2s evens (0,2,4..)
//#pragma unroll(I2S_MAX_DATALINES)
                for (size_t i = 0, idx = 0; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
                    port_out(p_dout[i], bitrev(out_samps[idx]));
                }
            }

            //Read lrclk value
            lrval = port_in(p_lrclk);

            //Input i2s evens (0,2,4..)
//#pragma unroll(I2S_MAX_DATALINES)
            for (size_t i = 0, idx = 0; i < num_in; i++, idx += I2S_CHANS_PER_FRAME) {
                int32_t data;
                data = port_in(p_din[i]);
                in_samps[idx] = bitrev(data);
            }

            syncerror += (lrval != expected_low);

            //Read lrclk value
            lrval = port_in(p_lrclk);

            //Output i2s odds (1,3,5..)
//#pragma unroll(I2S_MAX_DATALINES)
            if (num_out && (restart == I2S_NO_RESTART)) {
                for (size_t i = 0, idx = 1; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
                    port_out(p_dout[i], bitrev(out_samps[idx]));
                }
            }

            //Input i2s odds (1,3,5..)
//#pragma unroll(I2S_MAX_DATALINES)
            for (size_t i = 0, idx = 1; i < num_in; i++, idx += I2S_CHANS_PER_FRAME) {
                int32_t data;
                data = port_in(p_din[i]);
                in_samps[idx] = bitrev(data);
            }

            syncerror += (lrval != expected_high);

            if (num_in > 0)
                i2s_cbg->receive(i2s_cbg->app_data, num_in << 1, in_samps);
        } //main loop, runs until user restart or synch error
    }
}
