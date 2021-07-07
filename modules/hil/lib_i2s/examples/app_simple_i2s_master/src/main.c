// Copyright 2018-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* A simple application example used for code snippets in the library
 * documentation.
 */

#include <xs1.h>
#include <xcore/clock.h>
#include "i2s.h"

#define SAMPLE_FREQUENCY 192000
#define MASTER_CLOCK_FREQUENCY 24576000

void i2s_init(void *app_data, i2s_config_t *i2s_config)
{
    i2s_config->mclk_bclk_ratio = (MASTER_CLOCK_FREQUENCY/SAMPLE_FREQUENCY)/64;
    i2s_config->mode = I2S_MODE_LEFT_JUSTIFIED;
    // Complete setup
}

i2s_restart_t i2s_restart_check(void *app_data)
{
    // Inform the I2S slave whether it should restart or exit

    return I2S_NO_RESTART;
}

void i2s_receive(void *app_data, size_t num_in, int32_t *receive_data)
{
    // Handle a received frame
}

void i2s_send(void *app_data, size_t num_out, int32_t *send_data)
{
    // Provide a frame to send
}

port_t p_dout[2] = {XS1_PORT_1D, XS1_PORT_1E};
port_t p_din[2] = {XS1_PORT_1I, XS1_PORT_1J};
port_t p_bclk = XS1_PORT_1M;
port_t p_lrclk = XS1_PORT_1A;
port_t p_mclk = XS1_PORT_1B;

xclock_t bclk = XS1_CLKBLK_1;

int main(void)
{
    i2s_callback_group_t i2s_cbg = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = NULL,
    };

    port_enable(p_mclk);

    i2s_master(&i2s_cbg, p_dout, 2, p_din, 2, p_bclk, p_lrclk, p_mclk, bclk);

    return 0;
}
