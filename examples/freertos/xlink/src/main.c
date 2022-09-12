// Copyright 2019-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/triggerable.h>
#include <xcore/hwtimer.h>

#include <xs3a_registers.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"


// todo verify 1 == 5wire
#define W 0

#define RX_TIME_OUT_TICKS 500000000

// tx
#define RE_ENABLE_TX_PERIOD 6
#define SEND_CTRL_TOKEN 2500000

unsigned g_comm_state = 0;

#define RX_DIRECTION 0
#define TX_DIRECTION 5



#define INTER_DELAY 2
#define INTRA_DELAY 3
#define LINK_NUM 2

/*

tile 1
xlink2_rx4								IOR	X1D61
xlink2_rx3								IOR	X1D62
xlink2_rx2								IOR	X1D63

xlink2_rx1								IOR	X1D64
xlink2_rx0								IOR	X1D65
xlink2_tx0								IOR	X1D66
xlink2_tx1								IOR	X1D67

xlink2_tx2								IOR	X1D68
xlink2_tx3								IOR	X1D69
xlink2_tx4								IOR	X1D70

*/


void link_disable(unsigned tileid, unsigned link_num);
void link_enable(unsigned tileid, unsigned link_num);
void link_reset(unsigned tileid, unsigned link_num);
void link_hello(unsigned tileid, unsigned link_num);
unsigned link_got_credit(unsigned tileid, unsigned link_num);

/* Disable link by resetting ENABLE bit in link's control register */
void link_disable(unsigned tileid, unsigned link_num) {
    unsigned x = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, &x);
    x &= ~XS1_XLINK_ENABLE_MASK;
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, x);
}

/* Configure link by performing the following in the link's control register
 *   - Set intertoken and intratoken delays
 *   - Set ENABLE bit
 *   - Set WIDE bit if 5-bit link required
 */
void link_enable(unsigned tileid, unsigned link_num) {
    unsigned x = 0;
    (void) read_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, &x);
    x |= XS1_XLINK_INTER_TOKEN_DELAY_SET(x, INTER_DELAY);
    x |= XS1_XLINK_INTRA_TOKEN_DELAY_SET(x, INTRA_DELAY);
    x |= XS1_XLINK_ENABLE_MASK;
    x |= W * XS1_XLINK_WIDE_MASK;
    (void) write_sswitch_reg(tileid, XS1_SSWITCH_XLINK_0_NUM + link_num, x);
}

/* Reset link by setting RESET bit in link's control register */
void link_reset(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  x |= XS1_XLINK_RX_RESET_MASK;
  (void) write_sswitch_reg(tileid, l, x);
}

/* Send a HELLO by setting HELLO bit in link's control register */
void link_hello(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  x |= XS1_XLINK_HELLO_MASK;
  (void) write_sswitch_reg(tileid, l, x);
}

unsigned link_got_credit(unsigned tileid, unsigned link_num) {
  unsigned x = 0;
  unsigned l = XS1_SSWITCH_XLINK_0_NUM + link_num;
  (void) read_sswitch_reg(tileid, l, &x);
  return XS1_TX_CREDIT(x);
}





static int g_data_tokens = 0;
static int g_ctrl_tokens = 0;
static int g_timeout_cnts = 0;

#define RX_NODE_ID 0x20

void xlink_report_task(void) {
    int full_rep_cnt = 0;
    size_t n = 0;

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        rtos_i2c_master_write(i2c_master_ctx, 0xc, &g_data_tokens, sizeof(g_data_tokens), &n, 1);

        rtos_printf("Communication rate: %d bytes per sec \t\t\t ==>\t %d Mbit/sec\n", g_data_tokens, ((g_data_tokens*8)/1000000));
        if (!(full_rep_cnt++ % 10)) {
            rtos_printf("\nApplication control token count: \t\t%d \n", g_ctrl_tokens);
            rtos_printf("Receive timeouts: \t\t\t\t%d \n\n", g_timeout_cnts);
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

    size_t n = 0;

    rtos_printf("RX started...tile id: %x\n", get_local_tile_id());
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << 1));
    rtos_osal_thread_preemption_disable(NULL);

    while(1) {
        // rtos_printf("rx state: %d\n", comm_state);
        rtos_i2c_master_write(i2c_master_ctx, 0xa, &comm_state, sizeof(comm_state), &n, 1);
        switch (comm_state) {
            default:
                break;
            case 0: /* Setup link direction */
                reg_val = 0;
                direction = RX_DIRECTION;
                reg_val = XS1_LINK_DIRECTION_SET(reg_val, direction);
                (void) write_sswitch_reg(RX_NODE_ID, XS1_SSWITCH_SLINK_0_NUM + LINK_NUM, reg_val);
                comm_state = 1;
                break;
            case 1: /* Channel alloc */
                c_tileid = chanend_alloc();
                rtos_i2c_master_write(i2c_master_ctx, 0xf, &c_tileid, sizeof(c_tileid), &n, 1);
                comm_state = 2;
                break;
            case 2: /* Reconfigure links, setting up a single static link */
                for (int i=0; i<8; i++) {
                    link_disable(RX_NODE_ID, i);
                }
                link_enable(RX_NODE_ID, LINK_NUM);
                delay_milliseconds(100);
                x = 0;
                x |= XS1_XSTATIC_ENABLE_SET(x, 1);
                x |= XS1_XSTATIC_DEST_CHAN_END_SET(x, ((c_tileid >> 8) & 0x0000001F));
                x |= XS1_XSTATIC_DEST_PROC_SET(x, 1);

                (void) write_sswitch_reg(RX_NODE_ID, XS1_SSWITCH_XSTATIC_0_NUM + LINK_NUM, x);

                comm_state = 3;
                break;
            case 3: /* Wait for transmit credits */
                do {
                    link_reset(RX_NODE_ID, LINK_NUM);
                    link_hello(RX_NODE_ID, LINK_NUM);
                    delay_milliseconds(100);

                } while (!link_got_credit(RX_NODE_ID, LINK_NUM));

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

                uint32_t trigger_time = hwtimer_get_time(tmr_rx) + RX_TIME_OUT_TICKS;
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
                            trigger_time = hwtimer_get_time(tmr_rx) + RX_TIME_OUT_TICKS;
                            hwtimer_set_trigger_time(tmr_rx, trigger_time);
                            triggerable_enable_trigger(tmr_rx);
                            continue;
                        }
                        timeout:
                        {
                            triggerable_disable_trigger(c_tileid);
                            triggerable_disable_trigger(tmr_rx);
                            rtos_printf("Timed out\n");
                            rx_loop = 0;
                            comm_state = 3;
                            continue;
                        }
                    }
                }
                break;
            case 6: /* End of Communication */
                chanend_free(c_tileid);
                link_disable(RX_NODE_ID, LINK_NUM);
                comm_state = 1;
                break;
        }
    }
}

void xlink_tx_reenable(void) {
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(RE_ENABLE_TX_PERIOD * 1000));
        g_comm_state = 1;
        rtos_printf("Reenable tx link\n");
        link_disable(get_local_tile_id(), LINK_NUM);
        link_enable(get_local_tile_id(), LINK_NUM);
    }
}

void transmit_handler(unsigned comm_state) {
    chanend_t c_other_tile = 0;
    unsigned err_ctr = 0;
    int reg_val = 0;
    int direction = 0x0;
    int ret = 0;
    uint32_t id = 0;
    unsigned x = 0;

    unsigned tileid = get_local_tile_id();

    read_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_NODE_ID_NUM, &x);
    rtos_printf("node is 0x%x\n", x);

    while(1) {
        // rtos_printf("tx state: %d\n", comm_state);
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
                direction = TX_DIRECTION;
                reg_val = XS1_LINK_DIRECTION_SET(reg_val, direction);
                (void) write_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_SLINK_0_NUM + LINK_NUM, reg_val);
                comm_state = 1;
                break;
            case 1: /* Channel setup */
                c_other_tile = chanend_alloc();
                chanend_set_dest(c_other_tile, 0x00210902); // hardcode to expected rx

                (void) read_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_DIMENSION_DIRECTION1_NUM, &x);
                x = XS1_DIMF_DIR_SET(x, TX_DIRECTION);
                (void) write_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_DIMENSION_DIRECTION1_NUM, x);

                comm_state = 2;
                break;
            case 2: /* reconfigure links, leaving only one open */
                for (int i=0; i<8; i++) {
                    link_disable(tileid, i);
                }
                link_enable(tileid, LINK_NUM);
                comm_state = 3;
                break;
            case 3: /* Setup a static routing configuration */
                x = 0;
                x |= XS1_XSTATIC_ENABLE_SET(x, 1);
                x =  write_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_XSTATIC_0_NUM + LINK_NUM, x);

                delay_milliseconds(150);

                comm_state = 4;
                break;
            case 4: /* Wait for transmit credits */
                do {
                    link_reset(tileid, LINK_NUM);
                    link_hello(tileid, LINK_NUM);
                    delay_milliseconds(100);
                } while (!link_got_credit(tileid, LINK_NUM));
                // rtos_printf("TX Got Credit\n");

                /* Setup local control vars */
                err_ctr = 0;
                comm_state = 5;
                break;
            case 5: /* Send data tokens */
                chanend_out_byte(c_other_tile, 'a');
                
                if (err_ctr++ == SEND_CTRL_TOKEN) {
                    err_ctr = 0;
                    chanend_out_control_token(c_other_tile, XS1_CT_ACK);
                }
                break;
            case 6:
                chanend_free(c_other_tile);
                link_disable(tileid, LINK_NUM);
                comm_state = 2;
                break;
        }
    }
}

#include "trycatch.h"

void xlink_tx(void) {
    exception_t exception;

    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << 1));
    rtos_osal_thread_preemption_disable(NULL);

    rtos_printf("TX started...tile id: %x\n", get_local_tile_id());
    while(1) {
        TRY {
            transmit_handler(0);
        } CATCH (exception) {
            rtos_printf("Got exception.\n");
            transmit_handler(4);
        }
    }
}

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
    configASSERT(0);
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if 0
#if ON_TILE(1)
    vTaskDelay(pdMS_TO_TICKS(500));
#endif
    rtos_printf("\n\nDebug info:\n");
    rtos_printf("tile id: %x\n", get_local_tile_id());

    debug_print_sswitch_reg(XS1_SSWITCH_NODE_ID_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_NODE_CONFIG_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_DIMENSION_DIRECTION0_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_DIMENSION_DIRECTION1_NUM);
    // debug_print_sswitch_reg(XS1_SSWITCH_DEVICE_ID0_NUM);
    // debug_print_sswitch_reg(XS1_SSWITCH_DEVICE_ID1_NUM);
    // debug_print_sswitch_reg(XS1_SSWITCH_DEVICE_ID2_NUM);
    // debug_print_sswitch_reg(XS1_SSWITCH_DEVICE_ID3_NUM);

    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_0_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_1_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_2_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_3_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_4_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_5_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_6_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_7_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_SLINK_8_NUM);

    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_0_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_1_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_2_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_3_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_4_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_5_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_6_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_PLINK_7_NUM);

    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_0_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_1_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_2_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_3_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_4_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_5_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_6_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_7_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XLINK_8_NUM);

    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_0_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_1_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_2_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_3_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_4_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_5_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_6_NUM);
    debug_print_sswitch_reg(XS1_SSWITCH_XSTATIC_7_NUM);

    rtos_printf("\nDebug end\n\n");

#if ON_TILE(1)
    // _Exit(0);
#endif
    vTaskDelay(pdMS_TO_TICKS(500));
#endif
    rtos_printf("Create tasks\n");

#if ON_TILE(1)
#if DEMO_TILE == 0
    xTaskCreate((TaskFunction_t) xlink_report_task,
                "xlink_report_task",
                RTOS_THREAD_STACK_SIZE(xlink_report_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    xTaskCreate((TaskFunction_t) xlink_rx,
                "xlink_rx",
                RTOS_THREAD_STACK_SIZE(xlink_rx),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);
#else
    xTaskCreate((TaskFunction_t) xlink_tx_reenable,
                "xlink_tx_reenable",
                RTOS_THREAD_STACK_SIZE(xlink_tx_reenable),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    xTaskCreate((TaskFunction_t) xlink_tx,
                "xlink_tx",
                RTOS_THREAD_STACK_SIZE(xlink_tx),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);
#endif
#endif


#if ON_TILE(0)
port_t leds = XS1_PORT_4C;
int led_status=0;
port_enable(leds);
#endif
	for (;;) {
#if ON_TILE(0)
    port_out(leds, led_status);
    led_status ^= 1;
#endif
		// rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c0;
    (void)c2;
    (void)c3;

unsigned x = 0;
    read_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_NODE_ID_NUM, &x);
    rtos_printf("node is 0x%x\n", x);
#if DEMO_TILE == 0
    x = XS1_SS_NODE_ID_ID_SET(x, RX_NODE_ID);
    rtos_printf("set node to 0x%x\n", x);
    write_sswitch_reg_no_ack(get_local_tile_id(), XS1_SSWITCH_NODE_ID_NUM, x);
    delay_milliseconds(100);   

    chanend_t c_other = chanend_get_dest(c1);
    c_other &= 0x0000FFFF;
    c_other |= 0x00210000;
    chanend_set_dest(c1, c_other);
#endif

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c1;
    (void)c2;
    (void)c3;

#if DEMO_TILE == 0
    delay_milliseconds(110);   
    chanend_t c_other = chanend_get_dest(c0);
    c_other &= 0x0000FFFF;
    c_other |= 0x00200000;
    chanend_set_dest(c0, c_other);
#endif

    tile_common_init(c0);
}
#endif
