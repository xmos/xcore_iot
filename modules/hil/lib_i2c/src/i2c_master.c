// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xclib.h>
#include <xcore/triggerable.h>
#include <xcore/hwtimer.h>
#include <xcore/interrupt.h>
#include <xcore/assert.h>

#include "i2c.h"

#define SDA_LOW     0
#define SCL_LOW     0

#define BIT_TIME(KBITS_PER_SEC) ((XS1_TIMER_MHZ * 1000) / KBITS_PER_SEC)
#define BIT_MASK(BIT_POS) (1 << BIT_POS)

#define FOUR_POINT_SEVEN_MICRO_SECONDS_IN_TICKS     470
#define ONE_POINT_THREE_MICRO_SECONDS_IN_TICKS      130
#define JITTER_TICKS    3
#define WAKEUP_TICKS    10

inline uint32_t i2c_port_peek(resource_t p)
{
  uint32_t data;
  asm volatile("peek %0, res[%1]" : "=r" (data): "r" (p));
  return data;
}

inline uint32_t interrupt_state_get(void)
{
    uint32_t state;

    asm volatile(
        "getsr r11, %1\n"
        "mov %0, r11"
        : "=r"(state)
        : "n"(XS1_SR_IEBLE_MASK)
        : /* clobbers */ "r11"
    );

    return state;
}

inline void interrupt_disable(void)
{
    interrupt_mask_all();
}

inline void interrupt_restore(const i2c_master_t* ctx)
{
    if (ctx->interrupt_state) {
        interrupt_unmask_all();
    }
}

inline uint32_t wait_until(const i2c_master_t* ctx, resource_t t, uint32_t until) _XCORE_NOTHROW
{
  hwtimer_set_trigger_time(t, until);
  interrupt_restore(ctx);
  uint32_t now = hwtimer_get_time(t);
  interrupt_disable();
  hwtimer_clear_trigger_time(t);

  return now;
}

inline void wait_for(const i2c_master_t* ctx, resource_t t, uint32_t period) _XCORE_NOTHROW
{
  uint32_t start = hwtimer_get_time(t);
  uint32_t until = start + period;
  hwtimer_set_trigger_time(t, until);
  interrupt_restore(ctx);
  (void) hwtimer_get_time(t);
  interrupt_disable();
  hwtimer_clear_trigger_time(t);
}

/** Return the number of 10ns timer ticks required to meet the timing as defined
 *  in the standards.
 */
__attribute__((always_inline))
static inline uint32_t compute_low_period_ticks(
        const unsigned kbits_per_second)
{
    uint32_t ticks = 0;

    if (kbits_per_second <= 100) {
        ticks = FOUR_POINT_SEVEN_MICRO_SECONDS_IN_TICKS;
    } else if (kbits_per_second <= 400) {
        ticks = ONE_POINT_THREE_MICRO_SECONDS_IN_TICKS;
    } else {
        /* "Fast-mode Plus not implemented" */xassert(0);
    }

    // There is some jitter on the falling edges of the clock. In order to ensure
    // that the low period is respected we need to extend the minimum low period.
    return ticks + JITTER_TICKS;
}

__attribute__((always_inline))
static inline uint32_t compute_bus_off_ticks(
        const uint32_t bit_time)
{
    // Ensure the bus off time is respected. This is just over 1/2 bit time in
    // the case of the Fast-mode I2C so adding bit_time/16 ensures the timing
    // will be enforced
    return bit_time / 2 + bit_time / 16;
}

/** Reads back the SCL line, waiting until it goes high (in
 *  case the slave is clock stretching). It is assumed that the clock
 *  line has been release (driven high) before calling this function.
 *  Since the line going high may be delayed, the fall_time value may
 *  need to be adjusted
 */
static void wait_for_clock_high(
        const i2c_master_t* ctx,
        uint32_t* fall_time,
        uint32_t delay)
{
    const uint32_t scl_mask = ctx->scl_mask;
    const port_t p_scl = ctx->p_scl;
    uint32_t now;
    hwtimer_t tmr = ctx->tmr;

    interrupt_restore(ctx);
    while ((i2c_port_peek(p_scl) & scl_mask) == 0);
    interrupt_disable();

    now = wait_until(ctx, tmr, *fall_time + delay);

    // Adjust timing due to support clock stretching without clock drift in the
    // normal case.

    // If the time is beyond the time it takes simply to wake up and start
    // executing then the clock needs to be adjusted
    if (now > *fall_time + delay + WAKEUP_TICKS) {
        *fall_time = now - delay - WAKEUP_TICKS;
    }
}

static void high_pulse_drive(
        const i2c_master_t* ctx,
        int sda_value,
        uint32_t* fall_time)
{
    const uint32_t sda_low = ctx->sda_low;
    const uint32_t sda_high = ctx->sda_high;
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    const uint32_t bit_time = ctx->bit_time;
    const uint32_t three_quarter_bit_time = ctx->three_quarter_bit_time;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    hwtimer_t tmr = ctx->tmr;

    sda_value = sda_value ? sda_high : sda_low;

    if (p_scl == p_sda) {
        scl_low |= sda_value;
        scl_high |= sda_value;
        sda_value = scl_low;
    }

    port_out(p_sda, sda_value);
    port_out(p_scl, scl_low);
    (void) wait_until(ctx, tmr, *fall_time + low_period_ticks);
    port_out(p_scl, scl_high);
    wait_for_clock_high(ctx, fall_time, three_quarter_bit_time);
    *fall_time += bit_time;
    (void) wait_until(ctx, tmr, *fall_time);
    port_out(p_scl, scl_low);
}

/** 'Pulse' the clock line high and in the middle of the high period
 *  sample the data line (if required). Timing is done via the fall_time
 *  reference and bit_time period supplied.
 */
__attribute__((always_inline))
static inline uint32_t high_pulse_sample(
        const i2c_master_t *ctx,
        uint32_t *fall_time)
{
    const uint32_t sda_mask = ctx->sda_mask;
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    const hwtimer_t tmr = ctx->tmr;
    const uint32_t bit_time = ctx->bit_time;
    const uint32_t three_quarter_bit_time = ctx->three_quarter_bit_time;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_high = ctx->sda_high;

    uint32_t sample_value = 0;

    if (p_scl == p_sda) {
        scl_low |= sda_high;
        scl_high |= sda_high;
        sda_high = scl_low;
    }

    port_out(p_sda, sda_high);
    port_out(p_scl, scl_low);
    (void) wait_until(ctx, tmr, *fall_time + low_period_ticks);
    port_out(p_scl, scl_high);
    wait_for_clock_high(ctx, fall_time, three_quarter_bit_time);
    sample_value = (i2c_port_peek(p_sda) & sda_mask) ? 1 : 0;
    *fall_time += bit_time;
    (void) wait_until(ctx, tmr, *fall_time);
    port_out(p_scl, scl_low);

    return sample_value;
}

/** Output a start bit. The function returns the 'fall time' i.e. the
 *  reference clock time when the SCL line transitions to low.
 */
static void start_bit(
        const i2c_master_t *ctx,
        uint32_t *fall_time)
{
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    const hwtimer_t tmr = ctx->tmr;
    const uint32_t half_bit_time = ctx->half_bit_time;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    const uint32_t bus_off_ticks = ctx->bus_off_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_low = ctx->sda_low;
    uint32_t sda_high = ctx->sda_high;

    if (!ctx->stopped) {

        if (p_scl == p_sda) {
            scl_high |= sda_high;
            sda_high = scl_high;
        }

        (void) wait_until(ctx, tmr, *fall_time + low_period_ticks);

        port_out(p_scl, scl_high);
        port_out(p_sda, sda_high);
        wait_for_clock_high(ctx, fall_time, bus_off_ticks);
    }

    if (p_scl == p_sda) {
        scl_low |= sda_low;
        sda_low |= ctx->scl_high;
        scl_high = sda_low;
    }

    port_out(p_sda, sda_low);
    port_out(p_scl, scl_high);
    wait_for(ctx, tmr, half_bit_time);
    port_out(p_scl, scl_low);

    *fall_time = hwtimer_get_time(tmr);
}

/** Output a stop bit.
 */
static void stop_bit(
        const i2c_master_t *ctx,
        uint32_t *fall_time)
{
    const port_t p_scl = ctx->p_scl;
    const port_t p_sda = ctx->p_sda;
    const hwtimer_t tmr = ctx->tmr;
    const uint32_t bit_time = ctx->bit_time;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    const uint32_t bus_off_ticks = ctx->bus_off_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_low = ctx->sda_low;
    uint32_t sda_high = ctx->sda_high;

    if (p_scl == p_sda) {
        sda_high |= scl_high;
        scl_high |= sda_low;
        sda_low |= scl_low;
        scl_low = sda_low;
    }

    port_out(p_sda, sda_low);
    port_out(p_scl, scl_low);
    (void) wait_until(ctx, tmr, *fall_time + low_period_ticks);
    port_out(p_scl, scl_high);
    wait_for_clock_high(ctx, fall_time, bit_time);
    port_out(p_sda, sda_high);
    wait_for(ctx, tmr, bus_off_ticks);
}

/** Transmit 8 bits of data, then read the ack back from the slave and return
 *  that value.
 */
static uint32_t tx8(
        const i2c_master_t *ctx,
        uint32_t data,
        uint32_t *fall_time)
{
    // Data is transmitted MSB first
    data = bitrev(data) >> 24;
    for (size_t i = 8; i != 0; i--) {
        high_pulse_drive(ctx, data & 1, fall_time);
        data >>= 1;
    }
    return high_pulse_sample(ctx, fall_time);
}

i2c_res_t i2c_master_read(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    i2c_res_t result;
    const port_t p_scl = ctx->p_scl;
    const port_t p_sda = ctx->p_sda;
    const hwtimer_t tmr = ctx->tmr;
    const uint32_t bit_time = ctx->bit_time;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    const uint32_t three_quarter_bit_time = ctx->three_quarter_bit_time;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_high = ctx->sda_high;
    uint32_t fall_time = ctx->last_fall_time;

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    start_bit(ctx, &fall_time);

    uint32_t ack = tx8(ctx, (device_addr << 1) | 1, &fall_time);
    result = (ack == 0) ? I2C_ACK : I2C_NACK;

    if (p_scl == p_sda) {
        scl_low |= sda_high;
        sda_high |= scl_high;
    }

    if (result == I2C_ACK) {
        for (size_t j = 0; j < n; j++) {
            unsigned char data = 0;
            for (int i = 8; i != 0; i--) {
                uint32_t temp = high_pulse_sample(ctx, &fall_time);
                data = (data << 1) | temp;
            }
            buf[j] = data;

            uint32_t sda;
            if (j == n-1) {
              sda = ctx->sda_high;
            } else {
              sda = ctx->sda_low;
            }

            if (p_scl == p_sda) {
                sda |= ctx->scl_low;
                scl_high = ctx->scl_high | sda;
            }

            port_out(p_sda, sda);
            (void) wait_until(ctx, tmr, fall_time + low_period_ticks);
            port_out(p_scl, scl_high);
            wait_for_clock_high(ctx, &fall_time, three_quarter_bit_time);
            fall_time += bit_time;
            (void) wait_until(ctx, tmr, fall_time);
            port_out(p_sda, sda_high);
            port_out(p_scl, scl_low);
        }
    }

    if (send_stop_bit) {
        stop_bit(ctx, &fall_time);
        ctx->stopped = 1;
    } else {
        ctx->stopped = 0;
    }

    interrupt_restore(ctx);

    // Remember the last fall time to ensure the next start bit is valid
    ctx->last_fall_time = fall_time;

    return result;
}

i2c_res_t i2c_master_write(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    i2c_res_t result;
    uint32_t fall_time = ctx->last_fall_time;

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    start_bit(ctx, &fall_time);
    uint32_t ack = tx8(ctx, (device_addr << 1) | 0, &fall_time);

    size_t j = 0;
    for (; j < n && ack == 0; j++) {
        ack = tx8(ctx, buf[j], &fall_time);
    }

    if (send_stop_bit) {
        stop_bit(ctx, &fall_time);
        ctx->stopped = 1;
    } else {
        ctx->stopped = 0;
    }

    interrupt_restore(ctx);

    if (num_bytes_sent != NULL) {
        *num_bytes_sent = j;
    }

    result = (ack == 0) ? I2C_ACK : I2C_NACK;

    // Remember the last fall time to ensure the next start bit is valid
    ctx->last_fall_time = fall_time;

    return result;
}

void i2c_master_stop_bit_send(
        i2c_master_t *ctx)
{
    uint32_t fall_time = ctx->last_fall_time;

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    stop_bit(ctx, &fall_time);

    interrupt_restore(ctx);

    ctx->stopped = 1;

    // Remember the last fall time to ensure the next start bit is valid
    ctx->last_fall_time = fall_time;
}

void i2c_master_init(
        i2c_master_t *ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        hwtimer_t tmr,
        const unsigned kbits_per_second)
{
    memset(ctx, 0, sizeof(i2c_master_t));

    if (tmr != 0) {
        ctx->tmr = tmr;
    } else {
        ctx->tmr = hwtimer_alloc();
        xassert(ctx->tmr != 0);
        ctx->tmr_allocated = 1;
    }

    ctx->p_scl = p_scl;
    ctx->p_sda = p_sda;

    ctx->scl_mask = BIT_MASK(scl_bit_position);
    ctx->sda_mask = BIT_MASK(sda_bit_position);
    ctx->scl_high = ctx->scl_mask | scl_other_bits_mask;
    ctx->sda_high = ctx->sda_mask | sda_other_bits_mask;
    ctx->scl_low = scl_other_bits_mask;
    ctx->sda_low = sda_other_bits_mask;

    ctx->kbits_per_second = kbits_per_second;
    ctx->bit_time = BIT_TIME(kbits_per_second);
    ctx->three_quarter_bit_time = (ctx->bit_time * 3) / 4;
    ctx->half_bit_time = ctx->bit_time / 2;
    ctx->quarter_bit_time = ctx->bit_time / 4;
    ctx->low_period_ticks = compute_low_period_ticks(kbits_per_second);
    ctx->bus_off_ticks = compute_bus_off_ticks(ctx->bit_time);

    ctx->stopped = 1;
    ctx->last_fall_time = 0;

    port_enable(p_scl);
    port_write_control_word(p_scl, XS1_SETC_DRIVE_PULL_UP);

    if (p_scl == p_sda) {
        port_out(p_scl, ctx->scl_high | ctx->sda_high);
    } else {
        port_enable(p_sda);
        port_write_control_word(p_sda, XS1_SETC_DRIVE_PULL_UP);
        port_out(p_scl, ctx->scl_high);
        port_out(p_sda, ctx->sda_high);
    }
}

void i2c_master_shutdown(
        i2c_master_t *ctx)
{
    if (ctx->tmr_allocated) {
        hwtimer_free(ctx->tmr);
        ctx->tmr_allocated = 0;
    }

    if (ctx->p_sda != 0) {
        port_disable(ctx->p_sda);
    }
    if (ctx->p_scl != 0 && ctx->p_scl != ctx->p_sda) {
        port_disable(ctx->p_scl);
    }
    ctx->p_sda = 0;
    ctx->p_scl = 0;
}
