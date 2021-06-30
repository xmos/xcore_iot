// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xclib.h>
#include <xcore/port.h>
#include <xcore/clock.h>
#include <xcore/assert.h>

#include "i2s.h"

static void i2s_setup_bclk(
        xclock_t bclk,
        /*in*/port_t p_mclk,
        unsigned mclk_bclk_ratio)
{
    clock_enable(bclk);

    clock_set_source_port(bclk, p_mclk);
    clock_set_divide(bclk, mclk_bclk_ratio >> 1);
}

static void i2s_init_ports(
        const /*out buffered*/port_t /*:32*/p_dout[],
        const size_t num_out,
        const /*in buffered*/port_t /*:32*/p_din[],
        const size_t num_in,
        /*out*/port_t p_bclk,
        /*out buffered*/port_t /*:32*/p_lrclk,
        xclock_t bclk
        )
{
    size_t i;

    port_reset(p_bclk);
    port_set_clock(p_bclk, bclk);
    port_set_out_clock(p_bclk);

    port_start_buffered(p_lrclk, 32);
    port_set_clock(p_lrclk, bclk);
    port_out(p_lrclk, 1);

    for (i = 0; i < num_out; i++) {
        port_start_buffered(p_dout[i], 32);
        port_set_clock(p_dout[i], bclk);
        port_out(p_dout[i], 0);
    }

    for (i = 0; i < num_in; i++) {
        port_start_buffered(p_din[i], 32);
        port_set_clock(p_din[i], bclk);
    }
}

static void i2s_deinit_ports(
        const /*out buffered*/port_t /*:32*/p_dout[],
        const size_t num_out,
        const /*in buffered*/port_t /*:32*/p_din[],
        const size_t num_in,
        /*out*/port_t p_bclk,
        /*out buffered*/port_t /*:32*/p_lrclk
        )
{
    size_t i;

    port_disable(p_bclk);
    port_disable(p_lrclk);

    for (i = 0; i < num_out; i++) {
        port_disable(p_dout[i]);
    }

    for (i = 0; i < num_in; i++) {
        port_disable(p_din[i]);
    }
}

static i2s_restart_t i2s_ratio_n(
        const i2s_callback_group_t *const i2s_cbg,
        const port_t p_dout[],
        const size_t num_out,
        const port_t p_din[],
        const size_t num_in,
        const port_t p_bclk,
        const xclock_t bclk,
        const port_t p_lrclk,
        const i2s_mode_t mode)
{
    size_t i;
    size_t idx;
    int offset;

    /* two samples per data line, left and right */
    int32_t in_samps[I2S_CHANS_PER_FRAME * I2S_MAX_DATALINES];
    int32_t out_samps[I2S_CHANS_PER_FRAME * I2S_MAX_DATALINES];

    xassert(num_in <= I2S_MAX_DATALINES);
    xassert(num_out <= I2S_MAX_DATALINES);

    unsigned lr_mask = 0;

    for (i = 0; i < num_out; i++) {
        port_clear_buffer(p_dout[i]);
    }
    for (i = 0; i < num_in; i++) {
        port_clear_buffer(p_din[i]);
    }
    port_clear_buffer(p_lrclk);

    if (num_out > 0) {
        i2s_cbg->send(i2s_cbg->app_data, num_out << 1, out_samps);
    }

    //Start outputting evens (0,2,4..) data at correct point relative to the clock
    if (mode == I2S_MODE_I2S) {
        offset = 1;
    } else {
        offset = 0;
    }

//#pragma unroll(I2S_MAX_DATALINES)
    for (i = 0, idx = 0; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
        port_set_trigger_time(p_dout[i], 1 + offset);
        port_out(p_dout[i], bitrev(out_samps[idx]));
    }

    port_set_trigger_time(p_lrclk, 1);
    port_out(p_lrclk, lr_mask);

    clock_start(bclk);

    //And pre-load the odds (1,3,5..)
//#pragma unroll(I2S_MAX_DATALINES)
    for (i = 0, idx = 1; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
        port_out(p_dout[i], bitrev(out_samps[idx]));
    }

    lr_mask = ~lr_mask;
    port_out(p_lrclk, lr_mask);

    for (i = 0; i < num_in; i++) {
        port_set_trigger_time(p_din[i], 32 + offset);
    }

    for (;;) {
        // Check for restart
        i2s_restart_t restart = i2s_cbg->restart_check(i2s_cbg->app_data);

        if (restart == I2S_NO_RESTART) {
            if (num_out > 0) {
                i2s_cbg->send(i2s_cbg->app_data, num_out << 1, out_samps);
            }

            //Output i2s evens (0,2,4..)
//#pragma unroll(I2S_MAX_DATALINES)
            for (i = 0, idx = 0; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
                port_out(p_dout[i], bitrev(out_samps[idx]));
            }
        }

        //Input i2s evens (0,2,4..)
//#pragma unroll(I2S_MAX_DATALINES)
        for (i = 0, idx = 0; i < num_in; i++, idx += I2S_CHANS_PER_FRAME) {
            int32_t data;
            data = port_in(p_din[i]);
            in_samps[idx] = bitrev(data);
        }

        lr_mask = ~lr_mask;
        port_out(p_lrclk, lr_mask);

        if (restart == I2S_NO_RESTART) {
            //Output i2s odds (1,3,5..)
//#pragma unroll(I2S_MAX_DATALINES)
            for (i = 0, idx = 1; i < num_out; i++, idx += I2S_CHANS_PER_FRAME) {
                port_out(p_dout[i], bitrev(out_samps[idx]));
            }

            lr_mask = ~lr_mask;
            port_out(p_lrclk, lr_mask);
        }

        //Input i2s odds (1,3,5..)
//#pragma unroll(I2S_MAX_DATALINES)
        for (i = 0, idx = 1; i < num_in; i++, idx += I2S_CHANS_PER_FRAME) {
            int32_t data;
            data = port_in(p_din[i]);
            in_samps[idx] = bitrev(data);
        }

        if (num_in > 0) {
            i2s_cbg->receive(i2s_cbg->app_data, num_in << 1, in_samps);
        }

        if (restart != I2S_NO_RESTART) {
            if (num_in == 0) {
                // Prevent the clock from being stopped before the last word
                // has been sent if there are no RX ports.
                asm volatile("syncr res[%0]" : : "r" (p_dout[0]));
            }
            clock_stop(bclk);
            return restart;
        }
    }
    return I2S_RESTART;
}

void i2s_master(
        const i2s_callback_group_t *const i2s_cbg,
        const port_t p_dout[],
        const size_t num_out,
        const port_t p_din[],
        const size_t num_in,
        const port_t p_bclk,
        const port_t p_lrclk,
        const port_t p_mclk,
        const xclock_t bclk)
{
    for (;;) {
        i2s_config_t config;
        i2s_cbg->init(i2s_cbg->app_data, &config);

        if (!p_dout && !p_din) {
            xassert(0); /* Must provide non-null p_dout or p_din */
        }

        i2s_setup_bclk(bclk, p_mclk, config.mclk_bclk_ratio);

        //This ensures that the port time on all the ports is at 0
        i2s_init_ports(p_dout, num_out, p_din, num_in, p_bclk, p_lrclk, bclk);

        i2s_restart_t restart = i2s_ratio_n(i2s_cbg, p_dout, num_out, p_din,
                                             num_in,
                                             p_bclk, bclk, p_lrclk,
                                             config.mode);

        if (restart == I2S_SHUTDOWN) {
            i2s_deinit_ports(p_dout, num_out, p_din, num_in, p_bclk, p_lrclk);
            clock_disable(bclk);
            return;
        }
    }
}

void i2s_master_external_clock(
        const i2s_callback_group_t *const i2s_cbg,
        const port_t p_dout[],
        const size_t num_out,
        const port_t p_din[],
        const size_t num_in,
        const port_t p_bclk,
        const port_t p_lrclk,
        const xclock_t bclk)
{
    while (1) {
        i2s_config_t config;
        i2s_cbg->init(i2s_cbg->app_data, &config);

        if (!p_dout && !p_din) {
            xassert(0); /* Must provide non-null p_dout or p_din */
        }

        //This ensures that the port time on all the ports is at 0
        i2s_init_ports(p_dout, num_out, p_din, num_in, p_bclk, p_lrclk, bclk);

        i2s_restart_t restart = i2s_ratio_n(i2s_cbg, p_dout, num_out, p_din,
                                            num_in,
                                            p_bclk, bclk, p_lrclk,
                                            config.mode);

        if (restart == I2S_SHUTDOWN) {
            i2s_deinit_ports(p_dout, num_out, p_din, num_in, p_bclk, p_lrclk);
            return;
        }
    }
}
