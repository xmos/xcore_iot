// Copyright 2016-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xs1.h>
#include <i2s.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcore/parallel.h>

#ifndef NUM_I2S_LINES
#define NUM_I2S_LINES   2
#endif
#ifndef BURN_THREADS
#define BURN_THREADS    6
#endif
#ifndef SAMPLE_FREQUENCY
#define SAMPLE_FREQUENCY 768000
#endif
#ifndef TEST_LEN
#define TEST_LEN 1000
#endif
#ifndef RECEIVE_DELAY_INCREMENT
#define RECEIVE_DELAY_INCREMENT 5
#endif
#ifndef SEND_DELAY_INCREMENT
#define SEND_DELAY_INCREMENT 5
#endif

#ifndef GENERATE_MCLK
#define GENERATE_MCLK 0
#endif

#if GENERATE_MCLK
#define MASTER_CLOCK_FREQUENCY 25000000
#else
#define MASTER_CLOCK_FREQUENCY 24576000
#endif

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

/* Ports and clocks used by the application */
port_t p_lrclk = XS1_PORT_1G;
port_t p_bclk = XS1_PORT_1H;
port_t p_mclk = XS1_PORT_1F;
port_t p_dout[4] = {XS1_PORT_1M, XS1_PORT_1N, XS1_PORT_1O, XS1_PORT_1P};
port_t p_din [4] = {XS1_PORT_1I, XS1_PORT_1J, XS1_PORT_1K, XS1_PORT_1L};

xclock_t mclk = XS1_CLKBLK_1;
xclock_t bclk = XS1_CLKBLK_2;

static volatile int receive_delay = 5000;
static volatile int send_delay = 0;

void i2s_init(void *app_data, i2s_config_t *i2s_config)
{
    i2s_config->mode = I2S_MODE_I2S;
    i2s_config->mclk_bclk_ratio = (MASTER_CLOCK_FREQUENCY/SAMPLE_FREQUENCY)/64;
}

void i2s_send(void *app_data, size_t n, int32_t *send_data)
{
    for (size_t c = 0; c < n; c++) {
        send_data[c] = c;
    }
    if (send_delay) {
        delay_ticks(send_delay);
    }
}

void i2s_receive(void *app_data, size_t n, int32_t *receive_data)
{
    if (receive_delay) {
        delay_ticks(receive_delay);
    }
}

i2s_restart_t i2s_restart_check(void *app_data)
{
    return I2S_NO_RESTART;
}

#define OVERHEAD_TICKS 185 // Some of the period needs to be allowed for the callbacks
#define JITTER  1   //Allow for rounding so does not break when diff = period + 1
#define N_CYCLES_AT_DELAY   1 //How many LR clock cycles to measure at each backpressure delay value
#define DIFF_WRAP_16(new, old)  (new > old ? new - old : new + 0x10000 - old)
port_t p_lr_test = XS1_PORT_1A;
DECLARE_JOB(test_lr_period, (void));
void test_lr_period() {
    const int ref_tick_per_sample = XS1_TIMER_HZ/SAMPLE_FREQUENCY;
    const int period = ref_tick_per_sample;

    //set_core_fast_mode_on();
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    port_enable(p_lr_test);
    int time;

    // Synchronise with LR clock
    port_set_trigger_in_equal(p_lr_test, 1);
    (void) port_in(p_lr_test);
    port_set_trigger_in_equal(p_lr_test, 0);
    (void) port_in(p_lr_test);
    port_set_trigger_in_equal(p_lr_test, 1);
    (void) port_in(p_lr_test);
    port_set_trigger_in_equal(p_lr_test, 0);
    (void) port_in(p_lr_test);
    port_set_trigger_in_equal(p_lr_test, 1);
    (void) port_in(p_lr_test);
    time = port_get_trigger_time(p_lr_test);

    int time_old = time;
    int counter = 0; // Do a number cycles at each delay value
    while (1) {
        port_set_trigger_in_equal(p_lr_test, 0);
        (void) port_in(p_lr_test);
        counter++;
        if (counter == N_CYCLES_AT_DELAY) {
            receive_delay += RECEIVE_DELAY_INCREMENT;
            send_delay += SEND_DELAY_INCREMENT;
            if ((receive_delay + send_delay) > (period - OVERHEAD_TICKS)) {
                printf("PASS\n");
                _Exit(0);
            }
            counter = 0;
        }
        port_set_trigger_in_equal(p_lr_test, 1);
        (void) port_in(p_lr_test);
        time = port_get_trigger_time(p_lr_test);

        int diff = DIFF_WRAP_16(time, time_old);
        if (diff > (period + JITTER)) {
            printf("Backpressure breaks at receive delay ticks=%d, send delay ticks=%d\n",
            receive_delay, send_delay);
            printf("actual diff: %d, maximum (period + Jitter): %d\n",
            diff, (period + JITTER));
            _Exit(1);
        }
    time_old = time;
    }
}

DECLARE_JOB(burn, (void));

void burn(void) {
    //set_core_fast_mode_on();
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

int main() {
    i2s_callback_group_t i_i2s = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = NULL,
    };
    port_enable(p_bclk);
    port_enable(p_mclk);


#if GENERATE_MCLK
    // Generate a 25Mhz clock internally and drive p_mclk from that
    printf("Using divided reference clock\n");

    clock_enable(mclk);
    clock_set_source_clk_ref(mclk);
    clock_set_divide(mclk,2);   // 100 / 2*2 = 25Mhz
    port_set_clock(p_mclk, mclk);
    port_set_out_clock(p_mclk);
    clock_start(mclk);
#endif

  PAR_JOBS (
      PJOB(i2s_master, (
          &i_i2s,
          p_dout,
          NUM_I2S_LINES,
          p_din,
          NUM_I2S_LINES,
          p_bclk,
          p_lrclk,
          p_mclk,
          bclk)),

      PJOB(test_lr_period, ())
#if BURN_THREADS > 0
      ,
#endif

#if BURN_THREADS > 5
      PJOB(burn, ()),
#endif
#if BURN_THREADS > 4
      PJOB(burn, ()),
#endif
#if BURN_THREADS > 3
      PJOB(burn, ()),
#endif
#if BURN_THREADS > 2
      PJOB(burn, ()),
#endif
#if BURN_THREADS > 1
      PJOB(burn, ()),
#endif
#if BURN_THREADS > 0
      PJOB(burn, ())
#endif
  );
  return 0;
}
