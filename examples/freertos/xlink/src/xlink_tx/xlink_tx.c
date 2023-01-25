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
#include "trycatch.h"

/* App headers */
#include "app_conf.h"
#include "link_helpers.h"
#include "xlink_tx.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

static unsigned g_comm_state = 0;

void xlink_tx_reenable(void) {
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(appconfRE_ENABLE_TX_PERIOD * 1000));
        g_comm_state = 1;
        /* Reenable tx link */
        link_disable(get_local_tile_id(), appconfLINK_NUM);
        link_enable(get_local_tile_id(), appconfLINK_NUM);
    }
}

void transmit_handler(unsigned comm_state) {
    chanend_t c_other_tile = 0;
    unsigned err_ctr = 0;
    int reg_val = 0;
    int direction = 0x0;
    unsigned x = 0;

    unsigned switch_id = get_local_tile_id();

    while(1) {
        if (g_comm_state) {
            g_comm_state = 0;
            comm_state = 4;
        }
        switch (comm_state) {
            default:
                comm_state = 4;
                break;
            case 0: /* Setup Link Direction */
                reg_val = 0;
                direction = appconfTX_DIRECTION;
                reg_val = XS1_LINK_DIRECTION_SET(reg_val, direction);
                (void) write_sswitch_reg(switch_id, XS1_SSWITCH_SLINK_0_NUM + appconfLINK_NUM, reg_val);
                comm_state = 1;
                break;
            case 1: /* Channel setup */
                c_other_tile = chanend_alloc();
                chanend_set_dest(c_other_tile, 0x00210902); // hardcode to expected rx

                (void) read_sswitch_reg(switch_id, XS1_SSWITCH_DIMENSION_DIRECTION1_NUM, &x);
                x = XS1_DIMF_DIR_SET(x, appconfTX_DIRECTION);
                (void) write_sswitch_reg(switch_id, XS1_SSWITCH_DIMENSION_DIRECTION1_NUM, x);

                comm_state = 2;
                break;
            case 2: /* reconfigure links, leaving only one open */
                for (int i=0; i<8; i++) {
                    link_disable(switch_id, i);
                }
                link_enable(switch_id, appconfLINK_NUM);
                comm_state = 3;
                break;
            case 3: /* Setup a static routing configuration */
                x = 0;
                x |= XS1_XSTATIC_ENABLE_SET(x, 1);
                x =  write_sswitch_reg(switch_id, XS1_SSWITCH_XSTATIC_0_NUM + appconfLINK_NUM, x);

                delay_milliseconds(150);

                comm_state = 4;
                break;
            case 4: /* Wait for transmit credits */
                do {
                    link_reset(switch_id, appconfLINK_NUM);
                    link_hello(switch_id, appconfLINK_NUM);
                    delay_milliseconds(100);
                } while (!link_got_credit(switch_id, appconfLINK_NUM));

                /* Setup local control vars */
                err_ctr = 0;
                comm_state = 5;
                break;
            case 5: /* Send data tokens */
                chanend_out_byte(c_other_tile, 'a');
                
                if (err_ctr++ == appconfSEND_CTRL_TOKEN) {
                    err_ctr = 0;
                    chanend_out_control_token(c_other_tile, XS1_CT_ACK);
                }
                break;
            case 6:
                chanend_free(c_other_tile);
                link_disable(switch_id, appconfLINK_NUM);
                comm_state = 2;
                break;
        }
    }
}

void xlink_tx(void) {
    exception_t exception;

    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << appconfXLINK_TX_IO_CORE));
    rtos_osal_thread_preemption_disable(NULL);
    while(1) {
        TRY {
            transmit_handler(0);
        } CATCH (exception) {
            transmit_handler(4);
        }
    }
}

void create_xlink_tx_tasks(unsigned priority) {
    xTaskCreate((TaskFunction_t) xlink_tx_reenable,
                "xlink_tx_reenable",
                RTOS_THREAD_STACK_SIZE(xlink_tx_reenable),
                NULL,
                priority,
                NULL);

    xTaskCreate((TaskFunction_t) xlink_tx,
                "xlink_tx",
                RTOS_THREAD_STACK_SIZE(xlink_tx),
                NULL,
                priority,
                NULL);
}