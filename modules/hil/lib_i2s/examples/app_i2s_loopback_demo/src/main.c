// Copyright (c) 2014-2020, XMOS Ltd, All rights reserved
#include <platform.h>
#include "i2s.h"
#include <print.h>
#include <stdlib.h>

#define PORT_I2C_SCL XS1_PORT_1N
#define PORT_I2C_SDA XS1_PORT_1O

#define SAMPLE_FREQUENCY 192000
#define MASTER_CLOCK_FREQUENCY 24576000

/* Ports and clocks used by the application */
port_t p_lrclk = PORT_I2S_LRCLK;
port_t p_bclk = PORT_I2S_BCLK;
port_t p_mclk = PORT_MCLK_IN;
port_t p_dout[1] = {PORT_I2S_DAC_DATA};
port_t p_din[1] = {PORT_I2S_ADC_DATA};
port_t p_i2c_scl = PORT_I2C_SCL;
port_t p_i2c_sda = PORT_I2C_SDA;
port_t p_rst_shared = PORT_CODEC_RST_N;

xclock_t bclk = XS1_CLKBLK_1;

void i2s_init(void *app_data, i2s_config_t *i2s_config)
{
    i2s_config->mode = I2S_MODE_I2S;
    i2s_config->mclk_bclk_ratio = (MASTER_CLOCK_FREQUENCY/SAMPLE_FREQUENCY)/64;
}

i2s_restart_t i2s_restart_check(void *app_data)
{
    return I2S_NO_RESTART;
}

void i2s_receive(int32_t *samples, size_t num_chan_in, int32_t *sample)
{
    for (size_t i=0; i<num_chan_in; i++) {
        samples[i] = sample[i];
    }
}

void i2s_send(int32_t *samples, size_t num_chan_out, int32_t *sample)
{
    for (size_t i=0; i<num_chan_out; i++){
        sample[i] = samples[i];
    }
}

void codecs_init(void)
{
    /*
     * TODO: I2C CONFIG OF ADC/DAC.
     */
}

void main_tile0(void)
{
    codecs_init();
}

void main_tile1(void)
{
    int32_t samples[2] = {0, 0};
	
    i2s_callback_group_t i2s_cbg = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = samples,
    };

    i2s_master(&i2s_cbg, p_dout, 1, p_din, 1, p_bclk, p_lrclk, p_mclk, bclk);
}
