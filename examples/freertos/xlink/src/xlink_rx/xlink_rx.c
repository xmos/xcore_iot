// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/triggerable.h>
#include <xcore/hwtimer.h>
#include <xs3a_registers.h>

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "link_helpers.h"
#include "xlink_rx.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

static int g_data_tokens = 0;
static int g_ctrl_tokens = 0;
static int g_timeout_cnts = 0;

/* XLINK RX debug info */
#define RX_STATE_ID 0x01
#define RX_REPORT_BYTES_PER_SEC_ID 0x82
#define RX_REPORT_CTRL_TOKENS_PER_SEC_ID 0x83
#define RX_REPORT_TIMEOUTS_PER_SEC_ID 0x84

static void i2c_send_word(uint8_t id, uint32_t word) {
    uint8_t debug_buf[5] = {0};
    size_t n = 0;
    
    debug_buf[0] = id;
    memcpy(&debug_buf[1], &word, sizeof(uint32_t));
    rtos_i2c_master_write(i2c_master_ctx, appconfRX_DEBUG_I2C_SLAVE_ADDR, debug_buf, sizeof(debug_buf), &n, 1);
}

void xlink_report_task(void) {
    int full_rep_cnt = 0;
    uint8_t debug_buf[5] = {0};
    size_t n = 0;

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        i2c_send_word(RX_REPORT_BYTES_PER_SEC_ID, g_data_tokens);
        if (!(full_rep_cnt++ % 10)) {
            i2c_send_word(RX_REPORT_CTRL_TOKENS_PER_SEC_ID, g_ctrl_tokens);
            i2c_send_word(RX_REPORT_TIMEOUTS_PER_SEC_ID, g_timeout_cnts);
        }
        g_data_tokens = 0;
    }
}

void xlink_rx(void) {
    unsigned comm_state = 0;
    chanend_t c_tileid = 0;
    unsigned tm_out_ctr = 0;
    unsigned rx_loop = 0;

    uint32_t last_time = 0;
    hwtimer_t tmr_rx = hwtimer_alloc();
    char rx = 'z';
    uint32_t id = 0;
    int reg_val = 0;
    int direction = 0x0;

    unsigned x = 0;

    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << appconfXLINK_RX_IO_CORE));
    rtos_osal_thread_preemption_disable(NULL);

    while(1) {
        i2c_send_word(RX_STATE_ID, comm_state);

        switch (comm_state) {
            default:
                break;
            case 0: /* Setup link direction */
                reg_val = 0;
                direction = appconfRX_DIRECTION;
                reg_val = XS1_LINK_DIRECTION_SET(reg_val, direction);
                (void) write_sswitch_reg(appconfRX_NODE_ID, XS1_SSWITCH_SLINK_0_NUM + appconfLINK_NUM, reg_val);
                comm_state = 1;
                break;
            case 1: /* Channel alloc */
                c_tileid = chanend_alloc();
                comm_state = 2;
                break;
            case 2: /* Reconfigure links, setting up a single static link */
                for (int i=0; i<8; i++) {
                    link_disable(appconfRX_NODE_ID, i);
                }
                link_enable(appconfRX_NODE_ID, appconfLINK_NUM);
                delay_milliseconds(100);

                x = 0;
                x |= XS1_XSTATIC_ENABLE_SET(x, 1);
                x |= XS1_XSTATIC_DEST_CHAN_END_SET(x, ((c_tileid >> 8) & 0x0000001F));
                x |= XS1_XSTATIC_DEST_PROC_SET(x, 1);

                (void) write_sswitch_reg(appconfRX_NODE_ID, XS1_SSWITCH_XSTATIC_0_NUM + appconfLINK_NUM, x);

                delay_milliseconds(100);
                
                comm_state = 3;
                break;
            case 3: /* Wait for transmit credits */
                do {
                    link_reset(appconfRX_NODE_ID, appconfLINK_NUM);
                    link_hello(appconfRX_NODE_ID, appconfLINK_NUM);
                    delay_milliseconds(100);

                } while (!link_got_credit(appconfRX_NODE_ID, appconfLINK_NUM));

                /* Setup local control vars */
                rx_loop = 1;
                tm_out_ctr++;
                last_time = get_reference_time();
                comm_state = 4;
                break;
            case 4: /* Receive data loop */
                rx = 'z';

                TRIGGERABLE_SETUP_EVENT_VECTOR(tmr_rx, timeout);
                TRIGGERABLE_SETUP_EVENT_VECTOR(c_tileid, transaction);

                triggerable_disable_all();

                uint32_t trigger_time = hwtimer_get_time(tmr_rx) + appconfRX_TIME_OUT_TICKS;
                hwtimer_set_trigger_time(tmr_rx, trigger_time);
                triggerable_enable_trigger(tmr_rx);
                triggerable_enable_trigger(c_tileid);

                while(rx_loop) {
                    TRIGGERABLE_WAIT_EVENT(timeout, transaction);
                    {
                        transaction:
                        {
                            if (chanend_test_control_token_next_byte(c_tileid)) {
                                rx = chanend_in_control_token(c_tileid);
                                g_ctrl_tokens++;
                            } else {
                                rx = chanend_in_byte(c_tileid);
                                g_data_tokens++;
                            }
                            triggerable_disable_trigger(tmr_rx);
                            hwtimer_clear_trigger_time(tmr_rx);
                            trigger_time = hwtimer_get_time(tmr_rx) + appconfRX_TIME_OUT_TICKS;
                            hwtimer_set_trigger_time(tmr_rx, trigger_time);
                            triggerable_enable_trigger(tmr_rx);
                            continue;
                        }
                        timeout:
                        {
                            triggerable_disable_trigger(c_tileid);
                            triggerable_disable_trigger(tmr_rx);
                            rx_loop = 0;
                            comm_state = 3;
                            continue;
                        }
                    }
                }
                break;
            case 6: /* End of Communication */
                chanend_free(c_tileid);
                link_disable(appconfRX_NODE_ID, appconfLINK_NUM);
                comm_state = 1;
                break;
        }
    }
}

void create_xlink_rx_tasks(unsigned priority) {
    xTaskCreate((TaskFunction_t) xlink_report_task,
                "xlink_report_task",
                RTOS_THREAD_STACK_SIZE(xlink_report_task),
                NULL,
                priority,
                NULL);

    xTaskCreate((TaskFunction_t) xlink_rx,
                "xlink_rx",
                RTOS_THREAD_STACK_SIZE(xlink_rx),
                NULL,
                priority,
                NULL);
}