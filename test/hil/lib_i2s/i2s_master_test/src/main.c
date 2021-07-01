// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <i2s.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcore/parallel.h>

port_t p_mclk  = XS1_PORT_1A;
port_t p_bclk = XS1_PORT_1B;
port_t p_lrclk = XS1_PORT_1C;

port_t p_din [4] = {XS1_PORT_1D, XS1_PORT_1E, XS1_PORT_1F, XS1_PORT_1G};
port_t p_dout[4] = {XS1_PORT_1H, XS1_PORT_1I, XS1_PORT_1J, XS1_PORT_1K};

xclock_t bclk = XS1_CLKBLK_2;

port_t setup_strobe_port = XS1_PORT_1L;
port_t setup_data_port = XS1_PORT_16A;
port_t  setup_resp_port = XS1_PORT_1M;

#define MAX_RATIO 4

#define MAX_CHANNELS 8

#if defined(SMOKE)
#if NUM_OUT > 1 || NUM_IN > 1
#define NUM_MCLKS 1
static const unsigned mclock_freq[NUM_MCLKS] = {
        12288000,
};
#else
#define NUM_MCLKS 1
static const unsigned mclock_freq[NUM_MCLKS] = {
        24576000,
};
#endif
#else
#if NUM_OUT > 1 || NUM_IN > 1
#define NUM_MCLKS 2
static const unsigned mclock_freq[NUM_MCLKS] = {
        12288000,
        11289600,
};
#else
#define NUM_MCLKS 4
static const unsigned mclock_freq[NUM_MCLKS] = {
        24576000,
        22579200,
        12288000,
        11289600,
};
#endif
#endif

int32_t tx_data[MAX_CHANNELS][8] = {
        {  1,   2,   3,   4,   5,   6,   7,   8},
        {101, 102, 103, 104, 105, 106, 107, 108},
        {201, 202, 203, 204, 205, 206, 207, 208},
        {301, 302, 303, 304, 305, 306, 307, 308},
        {401, 402, 403, 404, 405, 406, 407, 408},
        {501, 502, 503, 504, 505, 506, 507, 508},
        {601, 602, 603, 604, 605, 606, 607, 608},
        {701, 702, 703, 704, 705, 706, 707, 708}};

int32_t rx_data[MAX_CHANNELS][8] = {
        {  1,   2,   3,   4,   5,   6,   7,   8},
        {101, 102, 103, 104, 105, 106, 107, 108},
        {201, 202, 203, 204, 205, 206, 207, 208},
        {301, 302, 303, 304, 305, 306, 307, 308},
        {401, 402, 403, 404, 405, 406, 407, 408},
        {501, 502, 503, 504, 505, 506, 507, 508},
        {601, 602, 603, 604, 605, 606, 607, 608},
        {701, 702, 703, 704, 705, 706, 707, 708}};

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

// __attribute__((always_inline))
// inline void port_sync(resource_t __p)
// {
//     asm volatile("syncr res[%0]" : : "r" (__p));
// }

static void send_data_to_tester(
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned data)
{
    port_out(setup_data_port, data);
    port_sync(setup_data_port);
    port_out(setup_strobe_port, 1);
    port_out(setup_strobe_port, 0);
    port_sync(setup_strobe_port);
}

static void broadcast(unsigned mclk_freq, unsigned mclk_bclk_ratio,
        unsigned num_in, unsigned num_out, int is_i2s_justified)
{
    port_out(setup_strobe_port, 0);
    send_data_to_tester(setup_strobe_port, setup_data_port, mclk_freq >> 16);
    send_data_to_tester(setup_strobe_port, setup_data_port, mclk_freq);
    send_data_to_tester(setup_strobe_port, setup_data_port, mclk_bclk_ratio);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_in);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_out);
    send_data_to_tester(setup_strobe_port, setup_data_port, is_i2s_justified);
}

static int request_response(
        port_t setup_strobe_port,
        port_t setup_resp_port)
{
    int r = 0;
    while (!r) {
        r = port_in(setup_resp_port);
    }

    port_out(setup_strobe_port, 1);
    port_out(setup_strobe_port, 0);
    for (int i = 0; i < 10 && r; i++, r = port_in(setup_resp_port));

    return r;
}

static int error=0;
static unsigned frames_sent = 0;
static unsigned rx_data_counter[MAX_CHANNELS] = {0};
static unsigned tx_data_counter[MAX_CHANNELS] = {0};

static int first_time = 1;
static unsigned mclock_freq_index=0;
static unsigned ratio_log2 = 1;
static i2s_mode_t current_mode = I2S_MODE_I2S;


void i2s_send(void *app_data, size_t n, int32_t *send_data)
{
    for(size_t c=0; c<n; c++){
        unsigned i = tx_data_counter[c];
        send_data[c] = tx_data[c][i];
        tx_data_counter[c] = i+1;
    }
}


void i2s_receive(void *app_data, size_t n, int32_t *receive_data)
{
    for(size_t c=0; c<n; c++){
        unsigned i = rx_data_counter[c];
        error |= (receive_data[c] != rx_data[c][i]);
        rx_data_counter[c] = i+1;
    }
}

i2s_restart_t i2s_restart_check(void *app_data)
{
    i2s_restart_t restart;

    frames_sent++;
    if (frames_sent == 4)
      restart = I2S_RESTART;
    else
      restart = I2S_NO_RESTART;

    return restart;
}

void i2s_init(void *app_data, i2s_config_t *i2s_config)
{
    if (!first_time) {
        unsigned x = request_response(setup_strobe_port, setup_resp_port);
        error |= x;
        if (error) {
            printf("Error: test fail\n");
        }

        int s = 0;
        while (!s) {
            if (ratio_log2 == MAX_RATIO) {
                ratio_log2 = 1;
                if (mclock_freq_index == NUM_MCLKS - 1) {
                    mclock_freq_index = 0;
                    if (current_mode == I2S_MODE_I2S) {
                        current_mode = I2S_MODE_LEFT_JUSTIFIED;
                    } else {
                        _Exit(1);
                    }
                } else {
                    mclock_freq_index++;
                }
            } else {
                ratio_log2++;
            }
            if (mclock_freq[mclock_freq_index] / ((1 << ratio_log2) * 64) <= 48000) {
                s = 1;
            }
        }
    }

    i2s_config->mclk_bclk_ratio = (1 << ratio_log2);

    frames_sent = 0;
    error = 0;
    first_time = 0;

    i2s_config->mode = current_mode;

    for (unsigned i = 0; i < MAX_CHANNELS; i++) {
        tx_data_counter[i] = 0;
        rx_data_counter[i] = 0;
    }

    broadcast(mclock_freq[mclock_freq_index],
              i2s_config->mclk_bclk_ratio,
              NUM_IN, NUM_OUT,
              i2s_config->mode == I2S_MODE_I2S);
}


DECLARE_JOB(spin, (void));

void spin(void)
{
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

int main(){
    i2s_callback_group_t i_i2s = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = NULL,
    };

    port_enable(setup_strobe_port);
    port_enable(setup_data_port);
    port_enable(setup_resp_port);
    port_enable(p_mclk);
    port_enable(p_bclk);

    PAR_JOBS (
        PJOB(i2s_master, (
            &i_i2s,
            p_dout,
            NUM_OUT,
            p_din,
            NUM_IN,
            p_bclk,
            p_lrclk,
            p_mclk,
            bclk)),

        PJOB(spin, ()),
        PJOB(spin, ()),
        PJOB(spin, ()),
        PJOB(spin, ()),
        PJOB(spin, ()),
        PJOB(spin, ()),
        PJOB(spin, ())
    );

    return 0;
}
