// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT ADAPTIVE_USB

#include "adaptive_rate_adjust.h"

#include <stdbool.h>
#include <xcore/port.h>
#include <rtos_printf.h>
#include <xscope.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>
#include <rtos_interrupt.h>

#include "platform/app_pll_ctrl.h"

int32_t dsp_math_multiply( int32_t input1_value, int32_t input2_value, int32_t q_format )
{
    int32_t ah; uint32_t al;
    int32_t result;
    // For rounding, accumulator is pre-loaded (1<<(q_format-1))
    asm("maccs %0,%1,%2,%3":"=r"(ah),"=r"(al):"r"(input1_value),"r"(input2_value),"0"(0),"1"(1<<(q_format-1)));
    asm("lextract %0,%1,%2,%3,32":"=r"(result):"r"(ah),"r"(al),"r"(q_format));
    return result;
}

#if XK_VOICE_L71
#define PORT_MCLK       PORT_MCLK_IN_OUT
#elif XCOREAI_EXPLORER
#define PORT_MCLK       PORT_MCLK_IN
#elif OSPREY_BOARD
#define PORT_MCLK       PORT_MCLK_IN
#else
#ifndef PORT_MCLK
#define PORT_MCLK       0
#endif
#endif

/*
 * TODO:
 * Technically, this is implementing synchronous mode, not adaptive mode.
 * If the number of audio frames sent per USB frame time is guaranteed to
 * always be the same (16 or 48) then the two modes are indistinguishable.
 * But if this is not the case (eg. occasionally receive 15 or 49) then
 * this method will not work to properly implement adaptive mode.
 *
 * If this function had visibility into the actual data rates then maybe
 * this can be solved here. Otherwise it may need to be moved into
 * tud_audio_rx_done_post_read_cb() or usb_audio_out_task(). These run
 * at a lower frequency though so would not be ideal.
 */

/*
 * Setting this non-zero initializes the appPLL numerator
 * to 0 to allow the PID controller's step response to be
 * viewed via xscope.
 */
#define PID_TEST 0

/*
 * 125 microseconds between each SOF at USB high speed.
 */
#define SOF_DELTA_TIME_US 125

/*
 * If the time between SOFs is off by more than
 * this, then reset the controller's error state.
 */
#define SOF_VALID_MAX_ERROR 0.05

/*
 * If the time between SOFs is off by more than
 * this, then assume it came in a little late or early
 * due to the interrupt being delayed or USB bus traffic,
 * and do not update the PID's output, but continue to
 * integrate the error.
 */
#define SOF_ON_TIME_MAX_ERROR 0.003

#define REFERENCE_CLOCK_TICKS_PER_MICROSECOND 100
#define US_TO_TICKS(us) (us * REFERENCE_CLOCK_TICKS_PER_MICROSECOND)

#define SOF_DELTA_TIME_NOMINAL US_TO_TICKS(SOF_DELTA_TIME_US)
#define SOF_DELTA_TIME_VALID_LOWER ((1.0 - SOF_VALID_MAX_ERROR) * SOF_DELTA_TIME_NOMINAL)
#define SOF_DELTA_TIME_VALID_UPPER ((1.0 + SOF_VALID_MAX_ERROR) * SOF_DELTA_TIME_NOMINAL)
#define SOF_DELTA_TIME_ON_TIME_LOWER ((1.0 - SOF_ON_TIME_MAX_ERROR) * SOF_DELTA_TIME_NOMINAL)
#define SOF_DELTA_TIME_ON_TIME_UPPER ((1.0 + SOF_ON_TIME_MAX_ERROR) * SOF_DELTA_TIME_NOMINAL)

/*
 * The ideal number of appPLL cycles between each SOF that
 * the PID controller is aiming for.
 */
#define DELTA_CYCLES_TARGET (((appconfAUDIO_CLOCK_FREQUENCY / 10) * SOF_DELTA_TIME_US) / 100000)

/*
 * The number of fractional bits in the fixed point
 * numbers used by the PID controller.
 */
#define P 16

__attribute__((dual_issue))
void sof_cb(void)
{
    static int32_t numerator = PID_TEST ? 0 : Q(P)((APP_PLL_FRAC_NOM & 0x0000FF00) >> 8);
    static int32_t previous_error;
    static int32_t integral;

    static uint32_t last_time;
    static uint16_t last_cycle_count;

    int valid;
    int on_time;

    uint16_t cur_cycle_count;
    uint16_t delta_cycles;
    uint32_t cur_time;
    uint32_t delta_time;

    if (PORT_MCLK == 0) {
        return;
    }

    asm volatile(
            "{gettime %0; getts %1, res[%2]}"
            : "=r"(cur_time), "=r"(cur_cycle_count)
            : "r"(PORT_MCLK)
            : /* no clobbers */
            );

    delta_time = cur_time - last_time;
    last_time = cur_time;
    valid = delta_time >= SOF_DELTA_TIME_VALID_LOWER && delta_time <= SOF_DELTA_TIME_VALID_UPPER;
    on_time = delta_time >= SOF_DELTA_TIME_ON_TIME_LOWER && delta_time <= SOF_DELTA_TIME_ON_TIME_UPPER;

    delta_cycles = cur_cycle_count - last_cycle_count;
    last_cycle_count = cur_cycle_count;

    if (valid) {

        int32_t error = DELTA_CYCLES_TARGET - delta_cycles;

        /*
         * Always integrate the error, even if this SOF is not on time.
         * Late SOFs are always followed by early SOFs, so the integrated
         * error will even out, even if the current error value is off.
         */
        integral += error;

        /*
         * If the current SOF came in either a little late or early (as
         * measured by the reference clock) then the instantaneous error
         * cannot be trusted, and no adjustment to the PLL frequency should
         * be made.
         */
        if (on_time) {
            const int32_t kp = Q(P)(0.1);
            const int32_t ki = Q(P)(0.0001);
            const int32_t kd = Q(P)(0);

            int32_t output;
            int32_t numerator_int;
            const int32_t proportional = error;
            const int32_t derivative = error - previous_error;
            previous_error = error;

            output = dsp_math_multiply(kp, proportional, 0) +
                     dsp_math_multiply(ki, integral, 0) +
                     dsp_math_multiply(kd, derivative, 0);

            numerator += output;
            if (numerator > Q(P)(255)) {
                numerator = Q(P)(255);
            } else if (numerator < 0) {
                numerator = 0;
            }

            numerator_int = (numerator + Q(P)(0.5)) >> P;

            output += Q(P)(0.5);
            output >>= P;
            rtos_printf("%u (%d, %d, %d) -> %d -> %d\n", delta_cycles,
                        proportional, integral, derivative,
                        output, numerator_int);

            app_pll_set_numerator(numerator_int);
            xscope_int(PLL_FREQ, numerator_int);
        } else {
            rtos_printf("no adjustment from PID\n");
        }

    } else {
        /*
         * The SOF arrived far too late or early. Reset the error state,
         * but leave the current PLL frequency alone. This might be due
         * to a USB reset for example.
         */
        previous_error = 0;
        integral = 0;
        rtos_printf("reset PID\n");
    }
}

static chanend_t sof_t1_isr_c;

bool tud_xcore_sof_cb(uint8_t rhport)
{
#if XCOREAI_EXPLORER
    sof_cb();
#else
    chanend_out_end_token(sof_t1_isr_c);
#endif

    /* False tells TinyUSB to not send the SOF event to the stack */
    return false;
}

DEFINE_RTOS_INTERRUPT_CALLBACK(sof_t1_isr, arg)
{
    (void) arg;

    chanend_check_end_token(sof_t1_isr_c);
    sof_cb();
}

#if XK_VOICE_L71 || OSPREY_BOARD
static void sof_intertile_init(chanend_t other_tile_c)
{
    sof_t1_isr_c = chanend_alloc();
    xassert(sof_t1_isr_c != 0);

#if ON_TILE(1)
    chanend_out_word(other_tile_c, sof_t1_isr_c);
    chanend_out_end_token(other_tile_c);
#endif
#if ON_TILE(0)
    chanend_set_dest(sof_t1_isr_c, chanend_in_word(other_tile_c));
    chanend_check_end_token(other_tile_c);
#endif

#if ON_TILE(1)
    /*
     * TODO: Move this to adaptive_rate_adjust_start() perhaps,
     * and then call it from platform_start().
     * It could then support enabling the ISR on a specified core.
     * ATM, the ISR will run on whatever core is running prior to
     * the RTOS starting, which isn't guaranteed to be anything,
     * though seems to always be RTOS core 0. This is fine, but
     * the tick interrupt can interfere, and if it does happen to
     * end up on the mic or i2s cores, that might be bad.
     */
    triggerable_setup_interrupt_callback(sof_t1_isr_c,
                                         NULL,
                                         RTOS_INTERRUPT_CALLBACK(sof_t1_isr));
    triggerable_enable_trigger(sof_t1_isr_c);
#endif
}
#endif

void adaptive_rate_adjust_init(chanend_t other_tile_c, xclock_t mclk_clkblk)
{
#if (XCOREAI_EXPLORER && ON_TILE(0)) || ((XK_VOICE_L71 || OSPREY_BOARD) && ON_TILE(1))
    /*
     * Configure the MCLK input port on the tile that
     * will run sof_cb() and count its clock cycles.
     *
     * On the Explorer board the appPLL/MCLK output from
     * tile 1 is wired over to tile 0. On the Osprey and
     * 3610 boards it is not.
     *
     * It is set up to clock itself. This allows GETTS to
     * be called on it to count its clock cycles. This
     * count is used to adjust its frequency to match the
     * USB host.
     */
    port_enable(PORT_MCLK);
    clock_enable(mclk_clkblk);
    clock_set_source_port(mclk_clkblk, PORT_MCLK);
    port_set_clock(PORT_MCLK, mclk_clkblk);
    clock_start(mclk_clkblk);
#endif
#if XK_VOICE_L71 || OSPREY_BOARD
    /*
     * On the Osprey and 3610 boards an additional intertile
     * channel and ISR must be set up in order to run the
     * SOF ISR on tile 1, since MCLK is not wired over to
     * tile 0.
     */
    sof_intertile_init(other_tile_c);
#endif
}
