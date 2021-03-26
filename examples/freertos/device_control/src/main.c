// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

#include "FreeRTOS.h"
#include "task.h"

#include "device_control.h"

#include "rtos_printf.h"

#include "board_init.h"

static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;

static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

#define GPIO_RPC_PORT 0
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

RTOS_I2C_SLAVE_CALLBACK_ATTR
void rtos_i2c_slave_rx_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    rtos_printf("I2C write %d bytes\n", len);
    for (int i = 0; i < len; i++) {
        rtos_printf("%02x ", data[i]);
    }
    rtos_printf("\n");

    if (len >= 3) {
        device_control_request(data[0],
                               data[1],
                               data[2]);

        device_control_payload_transfer(&data[3], len - 3, CONTROL_HOST_TO_DEVICE);
    }
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t rtos_i2c_slave_tx_start_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    device_control_payload_transfer(*data, RTOS_I2C_SLAVE_BUF_LEN, CONTROL_DEVICE_TO_HOST);
    rtos_printf("I2C read started\n");
    return 8;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void rtos_i2c_slave_tx_done_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    rtos_printf("I2C read of %d bytes complete\n", len);
}

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    rtos_intertile_start(
            intertile_ctx);

    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);


    #if ON_TILE(0)
    {
        rtos_printf("Starting GPIO driver\n");
        rtos_gpio_start(gpio_ctx);

        rtos_printf("Starting I2C slave driver\n");
        rtos_i2c_slave_start(i2c_slave_ctx,
                             NULL,
                             rtos_i2c_slave_rx_cb,
                             rtos_i2c_slave_tx_start_cb,
                             rtos_i2c_slave_tx_done_cb,
                             configMAX_PRIORITIES / 2);
    }
    #endif

    #if ON_TILE(USB_TILE_NO)
        //usb_manager_start(configMAX_PRIORITIES - 1);
    #endif

    chanend_free(other_tile_c);

    vTaskDelete(NULL);
}


void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{

    (void) c0;
    board_tile0_init(c1, intertile_ctx, i2c_slave_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                1,
                NULL);

    rtos_printf("start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx, gpio_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                configMAX_PRIORITIES-1,
                NULL);

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
