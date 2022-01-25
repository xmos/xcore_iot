// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library headers */
#include "rtos_printf.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

static chanend_t c_other_tile = 0;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
//		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

#define LOCAL_SYNC()    {chan_out_byte(c_other_tile, 0xA5); /*rtos_printf("local synced\n");*/}
#define REMOTE_SYNC()   {(void) chan_in_byte(c_other_tile);; /*rtos_printf("remote synced\n");*/}
void clock_controller_local_test_task(void *arg)
{
    unsigned tmp = 0;
    rtos_printf("Start local test task\n");
    vTaskDelay(pdMS_TO_TICKS(100));

    rtos_printf("\n** ref_clk_div tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_set_ref_clk_div to 6\n");
    rtos_clock_control_set_ref_clk_div(cc_ctx_t0, 6);
    rtos_printf("\tLocal done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("** ref_clk_div tests complete **\n");


    rtos_printf("\n** tile_clk_div tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_set_processor_clk_div to 100\n");
    rtos_clock_control_set_processor_clk_div(cc_ctx_t0, 100);
    rtos_printf("\tLocal done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("** tile_clk_div tests complete **\n");


    rtos_printf("\n** switch_clk_div tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_set_switch_clk_div to 1\n");
    rtos_clock_control_set_switch_clk_div(cc_ctx_t0, 1);
    rtos_printf("\tLocal done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("** tile_clk_div tests complete **\n");


    rtos_printf("\n** get various clock freq tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_ref_clock\n");
    tmp = rtos_clock_control_get_ref_clock(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_processor_clock\n");
    tmp = rtos_clock_control_get_processor_clock(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_switch_clock\n");
    tmp = rtos_clock_control_get_switch_clock(cc_ctx_t0);
    rtos_printf("\tLocal res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("** get various clock freq tests complete **\n");


    unsigned pre_div = 0;
    unsigned mul = 0;
    unsigned post_div = 0;
    rtos_printf("\n** node pll ratio tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tLocal pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tLocal pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_set_switch_clk_div to 1\n");
    rtos_clock_control_set_node_pll_ratio(cc_ctx_t0, 1, 50, 1);
    rtos_printf("\tLocal done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tLocal pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("** node pll ratio  tests complete **\n");


    #define DEFAULT_LINK_DELAY 3
    #define SCALED_LINK_DIVISOR 3
    #define SCALED_SWITCH_DIVISOR 100

    rtos_printf("\n** scale links tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_scale_links\n");
    rtos_clock_control_scale_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM,
            SCALED_LINK_DIVISOR * SCALED_SWITCH_DIVISOR,
            SCALED_LINK_DIVISOR * SCALED_SWITCH_DIVISOR);
    rtos_printf("\tLocal done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    // Return to full speed links
    rtos_printf("Local rtos_clock_control_scale_links\n");
    rtos_clock_control_scale_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM,
            DEFAULT_LINK_DELAY,
            DEFAULT_LINK_DELAY);
    rtos_printf("\tLocal done\n");
    // Reset credit on remote links
    rtos_printf("Local rtos_clock_control_reset_links\n");
    rtos_clock_control_reset_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM);
    rtos_printf("\tLocal done\n");
    LOCAL_SYNC();
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("** scale links tests complete **\n");


    rtos_printf("\n** local lock tests start **\n");
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_get_local_lock\n");
    rtos_clock_control_get_local_lock(cc_ctx_t0);
    rtos_printf("\tLocal done\n");
    rtos_printf("Local rtos_clock_control_get_local_lock\n");
    rtos_clock_control_get_local_lock(cc_ctx_t0);
    rtos_printf("\tLocal done\n");
    rtos_printf("Local owns lock twice\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    LOCAL_SYNC();
    rtos_printf("Local rtos_clock_control_release_local_lock\n");
    rtos_clock_control_release_local_lock(cc_ctx_t0);
    rtos_printf("\tLocal done\n");
    rtos_printf("Local rtos_clock_control_release_local_lock\n");
    rtos_clock_control_release_local_lock(cc_ctx_t0);
    rtos_printf("\tLocal done\n");
    rtos_printf("Local has given away lock\n");
    LOCAL_SYNC();
    LOCAL_SYNC();
    rtos_printf("** local lock tests complete **\n");

    _Exit(0);

    /* Done */
    vTaskDelete(NULL);
}

void clock_controller_remote_test_task(void *arg)
{
    unsigned tmp = 0;
    rtos_printf("Start remote test task\n");

    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_set_ref_clk_div to 5\n");
    rtos_clock_control_set_ref_clk_div(cc_ctx_t0, 5);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_ref_clk_div\n");
    tmp = rtos_clock_control_get_ref_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();


    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_set_processor_clk_div to 50\n");
    rtos_clock_control_set_processor_clk_div(cc_ctx_t0, 50);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_processor_clk_div\n");
    tmp = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();


    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_set_switch_clk_div to 2\n");
    rtos_clock_control_set_switch_clk_div(cc_ctx_t0, 2);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_switch_clk_div\n");
    tmp = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();


    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_ref_clock\n");
    tmp = rtos_clock_control_get_ref_clock(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_processor_clock\n");
    tmp = rtos_clock_control_get_processor_clock(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_switch_clock\n");
    tmp = rtos_clock_control_get_switch_clock(cc_ctx_t0);
    rtos_printf("\tRemote res:%d\n", tmp);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();

    unsigned pre_div = 0;
    unsigned mul = 0;
    unsigned post_div = 0;
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tRemote pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_set_switch_clk_div to 1\n");
    rtos_clock_control_set_node_pll_ratio(cc_ctx_t0, 2, 51, 2);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tRemote pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_node_pll_ratio\n");
    rtos_clock_control_get_node_pll_ratio(cc_ctx_t0, &pre_div, &mul, &post_div);
    rtos_printf("\tRemote pre:%d mul:%d post:%d\n", pre_div, mul, post_div);
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();


    REMOTE_SYNC();
    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_scale_links\n");
    rtos_clock_control_scale_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM,
            SCALED_LINK_DIVISOR * SCALED_SWITCH_DIVISOR,
            SCALED_LINK_DIVISOR * SCALED_SWITCH_DIVISOR);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();
    // Return to full speed links
    rtos_printf("Remote rtos_clock_control_scale_links\n");
    rtos_clock_control_scale_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM,
            DEFAULT_LINK_DELAY,
            DEFAULT_LINK_DELAY);
    rtos_printf("\tRemote done\n");
    // Reset credit on remote links
    rtos_printf("Remote rtos_clock_control_reset_links\n");
    rtos_clock_control_reset_links(
            cc_ctx_t0,
            XS1_SSWITCH_XLINK_0_NUM,
            XS1_SSWITCH_XLINK_1_NUM);
    rtos_printf("\tRemote done\n");
    vTaskDelay(pdMS_TO_TICKS(100));
    REMOTE_SYNC();


    REMOTE_SYNC();
    REMOTE_SYNC();
    rtos_printf("Remote rtos_clock_control_get_local_lock\n");
    rtos_clock_control_get_local_lock(cc_ctx_t0);
    rtos_printf("\tRemote done\n");
    rtos_printf("Remote owns lock\n");
    REMOTE_SYNC();
    vTaskDelay(pdMS_TO_TICKS(100));
    rtos_printf("Remote rtos_clock_control_release_local_lock\n");
    rtos_clock_control_release_local_lock(cc_ctx_t0);
    rtos_printf("\tRemote done\n");
    REMOTE_SYNC();

    /* Done */
    vTaskDelete(NULL);
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

#if ON_TILE(0)
    xTaskCreate((TaskFunction_t) clock_controller_local_test_task,
                "clock_controller_local_test_task",
                RTOS_THREAD_STACK_SIZE(clock_controller_local_test_task),
                NULL,
                5,
                NULL);
#endif

#if ON_TILE(1)
    xTaskCreate((TaskFunction_t) clock_controller_remote_test_task,
                "clock_controller_remote_test_task",
                RTOS_THREAD_STACK_SIZE(clock_controller_remote_test_task),
                NULL,
                5,
                NULL);
#endif

    platform_start();

    mem_analysis();
    /*
     * TODO: Watchdog?
     */
}

void vApplicationMinimalIdleHook(void)
{
    asm volatile("waiteu");
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    c_other_tile = c;

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
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
