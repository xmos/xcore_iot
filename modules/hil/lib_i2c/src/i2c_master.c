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

extern uint8_t read_reg(i2c_master_t *ctx, uint8_t device_addr, uint8_t reg, i2c_regop_res_t *result);
extern uint8_t read_reg8_addr16(i2c_master_t *ctx, uint8_t device_addr, uint16_t reg, i2c_regop_res_t *result);
extern uint16_t read_reg16_addr8(i2c_master_t *ctx, uint8_t device_addr, uint8_t reg, i2c_regop_res_t *result);
extern uint16_t read_reg16(i2c_master_t *ctx, uint8_t device_addr, uint16_t reg, i2c_regop_res_t *result);
extern i2c_regop_res_t write_reg(i2c_master_t *ctx, uint8_t device_addr, uint8_t reg, uint8_t data);
extern i2c_regop_res_t write_reg8_addr16(i2c_master_t *ctx, uint8_t device_addr, uint16_t reg, uint8_t data);
extern i2c_regop_res_t write_reg16_addr8(i2c_master_t *ctx, uint8_t device_addr, uint8_t reg, uint16_t data);
extern i2c_regop_res_t write_reg16(i2c_master_t *ctx, uint8_t device_addr, uint16_t reg, uint16_t data);

#define SDA_LOW     0
#define SCL_LOW     0

#define BIT_TIME(KBITS_PER_SEC) ((XS1_TIMER_MHZ * 1000) / KBITS_PER_SEC)
#define BIT_MASK(BIT_POS) (1 << BIT_POS)

#define JITTER_TICKS    3
#define WAKEUP_TICKS    20

static uint32_t interrupt_state_get(void)
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

__attribute__((always_inline))
static inline void interrupt_disable(void)
{
    interrupt_mask_all();
}

__attribute__((always_inline))
static inline void interrupt_restore(const i2c_master_t *ctx)
{
    if (ctx->interrupt_state) {
        interrupt_unmask_all();
    }
}

__attribute__((always_inline))
static inline void hold_port_value_for(port_t p, uint32_t val, uint32_t t)
{
    port_sync(p);
    port_out_at_time(p, port_get_trigger_time(p) + t, val);
}

__attribute__((always_inline))
static inline uint32_t wait_for_clock_high(
        const i2c_master_t *ctx,
        uint32_t scl_val)
{
    const uint32_t scl_mask = ctx->scl_mask;
    const port_t p_scl = ctx->p_scl;

    interrupt_restore(ctx);
    port_sync(p_scl);
    while ((port_peek(p_scl) & scl_mask) == 0);
    interrupt_disable();
    port_out(p_scl, scl_val);
    port_sync(p_scl);
    return port_get_trigger_time(p_scl);
}

__attribute__((always_inline))
static inline void high_pulse_drive(
        const i2c_master_t *ctx,
        int sda_value)
{
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint16_t scheduled_fall_time;
    uint16_t actual_fall_time;
    uint16_t rise_time;

    sda_value = sda_value ? ctx->sda_high : ctx->sda_low;

    if (p_scl == p_sda) {
        scl_low |= sda_value;
        scl_high |= sda_value;
        sda_value = scl_low;
    }

    interrupt_restore(ctx);
    port_sync(p_scl);
    interrupt_disable();
    scheduled_fall_time = port_get_trigger_time(p_scl);
    port_out(p_scl, scl_low);
    port_out(p_sda, sda_value);

    port_sync(p_scl);
    actual_fall_time = port_get_trigger_time(p_scl);
    port_out_at_time(p_scl, actual_fall_time + ctx->low_period_ticks, scl_high);
    rise_time = wait_for_clock_high(ctx, scl_high);

    scheduled_fall_time += ctx->bit_time;
    if ((int16_t) (scheduled_fall_time - actual_fall_time) < ctx->bit_time - WAKEUP_TICKS) {
        scheduled_fall_time = actual_fall_time + ctx->bit_time;
    }
    if ((int16_t) (scheduled_fall_time - rise_time) < ctx->high_period_ticks) {
        scheduled_fall_time = rise_time + ctx->high_period_ticks;
    }

    port_out_at_time(p_scl, scheduled_fall_time, scl_high);
}

__attribute__((always_inline))
static inline uint32_t high_pulse_sample(
        const i2c_master_t *ctx)
{
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_high = ctx->sda_high;
    uint16_t scheduled_fall_time;
    uint16_t actual_fall_time;
    uint16_t rise_time;

    uint32_t sample_value;

    if (p_scl == p_sda) {
        scl_low |= sda_high;
        scl_high |= sda_high;
        sda_high = scl_low;
    }

    interrupt_restore(ctx);
    port_sync(p_scl);
    interrupt_disable();
    scheduled_fall_time = port_get_trigger_time(p_scl);
    port_out(p_scl, scl_low);
    port_out(p_sda, sda_high);

    port_sync(p_scl);
    actual_fall_time = port_get_trigger_time(p_scl);
    port_out_at_time(p_scl, actual_fall_time + ctx->low_period_ticks, scl_high);
    rise_time = wait_for_clock_high(ctx, scl_high);

    sample_value = (port_peek(p_sda) & ctx->sda_mask) ? 1 : 0;

    scheduled_fall_time += ctx->bit_time;
    if ((int16_t) (scheduled_fall_time - actual_fall_time) < ctx->bit_time - WAKEUP_TICKS) {
        scheduled_fall_time = actual_fall_time + ctx->bit_time;
    }
    if ((int16_t) (scheduled_fall_time - rise_time) < ctx->high_period_ticks) {
        scheduled_fall_time = rise_time + ctx->high_period_ticks;
    }

    port_out_at_time(p_scl, scheduled_fall_time, scl_high);

    return sample_value;
}

static void start_bit(
        const i2c_master_t *ctx)
{
    const port_t p_sda = ctx->p_sda;
    const port_t p_scl = ctx->p_scl;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    const uint32_t s_hold_ticks = ctx->s_hold_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_low = ctx->sda_low;
    uint32_t sda_high = ctx->sda_high;

    if (!ctx->stopped) {

        const uint32_t sr_setup_ticks = ctx->sr_setup_ticks;

        if (p_scl == p_sda) {
            scl_low |= sda_high;
            scl_high |= sda_high;
            sda_high = scl_high;
        }

        /*
         * When a transaction does not end with a stop bit, SCL is left low and
         * SDA is left high. Ensure SCL is held low for the minimum low period.
         */
        hold_port_value_for(p_scl, scl_low, low_period_ticks);
        port_sync(p_scl);

        port_out(p_scl, scl_high);
        uint32_t rise_time = wait_for_clock_high(ctx, scl_high);
        /*
         * Output SDA high here at the time at which SDA should go
         * low. This is so that the sync below will wait the correct
         * amount of time. The sync below is also used to ensure the
         * bus off time if a stop bit was previously sent.
         */
        port_out_at_time(p_sda, rise_time + sr_setup_ticks, sda_high);
    }

    if (p_scl == p_sda) {
        scl_low |= sda_low;
        sda_low |= ctx->scl_high;
        scl_high = sda_low;
    }

    /*
     * The previous stop bit (or the repeated start preparation above)
     * outputs high at the time at which SDA may go low. Sync here to
     * ensure that time is reached before continuing on.
     */
    port_sync(p_sda);

    port_out(p_sda, sda_low);
    port_sync(p_sda);
    port_out_at_time(p_scl, port_get_trigger_time(p_sda) + s_hold_ticks, scl_high);
}

/** Output a stop bit.
 */
static void stop_bit(
        const i2c_master_t *ctx)
{
    const port_t p_scl = ctx->p_scl;
    const port_t p_sda = ctx->p_sda;
    const uint32_t low_period_ticks = ctx->low_period_ticks;
    const uint32_t bus_off_ticks = low_period_ticks; /* bus off and scl low times are the same */
    const uint32_t p_setup_ticks = ctx->p_setup_ticks;
    uint32_t scl_low = ctx->scl_low;
    uint32_t scl_high = ctx->scl_high;
    uint32_t sda_low = ctx->sda_low;
    uint32_t sda_high = ctx->sda_high;

    if (p_scl == p_sda) {
        sda_high |= scl_high;
        scl_high |= sda_low;
        scl_low |= sda_low;
    }

    /*
     * Both SCL must be low before stop_bit() is called.
     * Ensure SCL is held low for at least low_period ticks
     * outputting its rising edge.
     */
    hold_port_value_for(p_scl, scl_low, low_period_ticks);
    port_sync(p_scl);

    port_out(p_scl, scl_high);
    wait_for_clock_high(ctx, scl_high);
    hold_port_value_for(p_scl, scl_high, p_setup_ticks);
    port_sync(p_scl);

    port_out(p_sda, sda_high);
    hold_port_value_for(p_sda, sda_high, bus_off_ticks);
}

__attribute__((always_inline))
static inline uint32_t tx8(
        const i2c_master_t *ctx,
        uint32_t data)
{
    // Data is transmitted MSB first
    data = bitrev(data) >> 24;
    for (size_t i = 8; i != 0; i--) {
        high_pulse_drive(ctx, data & 1);
        data >>= 1;
    }
    return high_pulse_sample(ctx);
}

i2c_res_t i2c_master_read(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    i2c_res_t result;
    uint32_t scl_low;
    uint32_t sda_low;

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    start_bit(ctx);

    uint32_t ack = tx8(ctx, (device_addr << 1) | 1);
    result = (ack == 0) ? I2C_ACK : I2C_NACK;

    if (result == I2C_ACK) {
        for (size_t j = 0; j < n; j++) {
            uint8_t data = 0;
            for (int i = 8; i != 0; i--) {
                uint32_t temp = high_pulse_sample(ctx);
                data = (data << 1) | temp;
            }
            buf[j] = data;

            uint32_t sda_value;
            if (j == n-1) {
                sda_value = 1;
            } else {
                sda_value = 0;
            }

            high_pulse_drive(ctx, sda_value);
        }
    }

    scl_low = ctx->scl_low;
    sda_low = ctx->sda_low;

    if (ctx->p_scl == ctx->p_sda) {
        sda_low |= scl_low;
        scl_low |= ctx->sda_high;
    }

    interrupt_disable();
    port_sync(ctx->p_scl);
    interrupt_restore(ctx);
    port_out(ctx->p_scl, scl_low);

    if (send_stop_bit) {
        port_out(ctx->p_sda, sda_low);
        stop_bit(ctx);
        ctx->stopped = 1;
    } else {
        ctx->stopped = 0;
    }

    interrupt_restore(ctx);

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
    uint32_t scl_low;
    uint32_t sda_low;

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    start_bit(ctx);
    uint32_t ack = tx8(ctx, (device_addr << 1) | 0);

    size_t j = 0;
    for (; j < n && ack == 0; j++) {
        ack = tx8(ctx, buf[j]);
    }

    scl_low = ctx->scl_low;
    sda_low = ctx->sda_low;

    if (ctx->p_scl == ctx->p_sda) {
        sda_low |= scl_low;
        scl_low |= ctx->sda_high;
    }

    interrupt_disable();
    port_sync(ctx->p_scl);
    interrupt_restore(ctx);
    port_out(ctx->p_scl, scl_low);

    if (send_stop_bit) {
        port_out(ctx->p_sda, sda_low);
        stop_bit(ctx);
        ctx->stopped = 1;
    } else {
        ctx->stopped = 0;
    }

    interrupt_restore(ctx);

    if (num_bytes_sent != NULL) {
        *num_bytes_sent = j;
    }

    result = (ack == 0) ? I2C_ACK : I2C_NACK;

    return result;
}

void i2c_master_stop_bit_send(
        i2c_master_t *ctx)
{
    const port_t p_scl = ctx->p_scl;
    const port_t p_sda = ctx->p_sda;
    uint32_t scl_low = ctx->scl_low;
    uint32_t sda_low = ctx->sda_low;

    if (p_scl == p_sda) {
        sda_low |= scl_low;
    }

    ctx->interrupt_state = interrupt_state_get();
    interrupt_disable();

    port_out(p_sda, sda_low);
    stop_bit(ctx);

    interrupt_restore(ctx);

    ctx->stopped = 1;
}

void i2c_master_init(
        i2c_master_t *ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        const unsigned kbits_per_second)
{
    memset(ctx, 0, sizeof(i2c_master_t));

    ctx->p_scl = p_scl;
    ctx->p_sda = p_sda;

    ctx->scl_mask = BIT_MASK(scl_bit_position);
    ctx->sda_mask = BIT_MASK(sda_bit_position);
    ctx->scl_high = ctx->scl_mask | scl_other_bits_mask;
    ctx->sda_high = ctx->sda_mask | sda_other_bits_mask;
    ctx->scl_low = scl_other_bits_mask;
    ctx->sda_low = sda_other_bits_mask;

    ctx->bit_time = BIT_TIME(kbits_per_second);

    if (kbits_per_second <= 100) {
        ctx->low_period_ticks = 470 + JITTER_TICKS;
        ctx->high_period_ticks = 100 + 400 + JITTER_TICKS; /* max rise time plus min high period */
        ctx->p_setup_ticks = 400 + JITTER_TICKS;
        ctx->sr_setup_ticks = 470 + JITTER_TICKS;
        ctx->s_hold_ticks = 400 + JITTER_TICKS;
    } else if (kbits_per_second <= 400) {
        ctx->low_period_ticks = 130 + JITTER_TICKS;
        ctx->high_period_ticks = 30 + 60 + JITTER_TICKS; /* max rise time plus min high period */
        ctx->p_setup_ticks = 60 + JITTER_TICKS;
        ctx->sr_setup_ticks = 60 + JITTER_TICKS;
        ctx->s_hold_ticks = 60 + JITTER_TICKS;
    } else {
        /* "Fast-mode Plus not implemented" */xassert(0);
    }

    ctx->stopped = 1;

    /*
     * Both ports are enabled at the same time here without checking
     * to see if they are different ports, so that if they are their
     * port times will be as close as possible.
     */
    port_enable(p_scl);
    port_enable(p_sda);
    port_write_control_word(p_scl, XS1_SETC_DRIVE_PULL_UP);

    if (p_scl == p_sda) {
        port_out(p_scl, ctx->scl_high | ctx->sda_high);
    } else {
        port_write_control_word(p_sda, XS1_SETC_DRIVE_PULL_UP);
        port_out(p_scl, ctx->scl_high);
        port_out(p_sda, ctx->sda_high);
    }
}

void i2c_master_shutdown(
        i2c_master_t *ctx)
{
    if (ctx->p_sda != 0) {
        port_disable(ctx->p_sda);
    }
    if (ctx->p_scl != 0 && ctx->p_scl != ctx->p_sda) {
        port_disable(ctx->p_scl);
    }
    ctx->p_sda = 0;
    ctx->p_scl = 0;
}
