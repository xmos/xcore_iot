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
static device_control_t device_control_ctx_s;

static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static device_control_t *device_control_ctx = &device_control_ctx_s;

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

#define GPIO_RPC_PORT 0
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)

RTOS_I2C_SLAVE_CALLBACK_ATTR
void rtos_i2c_slave_start_cb(rtos_i2c_slave_t *ctx, void *app_data)
{
    rtos_printf("I2C STARTED\n");

    control_ret_t dc_ret;

    dc_ret = device_control_resources_register(device_control_ctx,
                                               2, //SERVICER COUNT
                                               pdMS_TO_TICKS(100));

    if (dc_ret != CONTROL_SUCCESS) {
        rtos_printf("Device control resources failed to register on tile %d\n", THIS_XCORE_TILE);
    } else {
        rtos_printf("Device control resources registered on tile %d\n", THIS_XCORE_TILE);
    }
    xassert(dc_ret == CONTROL_SUCCESS);
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void rtos_i2c_slave_rx_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    control_ret_t ret;

    if (len >= 3) {
        device_control_request(device_control_ctx,
                               data[0],
                               data[1],
                               data[2]);

        ret = device_control_payload_transfer(device_control_ctx,
                                              &data[3], len - 3, CONTROL_HOST_TO_DEVICE);
        rtos_printf("I2C write completed - device control status %d\n", ret);
    }
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t rtos_i2c_slave_tx_start_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    control_ret_t ret;

    ret = device_control_payload_transfer(device_control_ctx,
                                          *data, RTOS_I2C_SLAVE_BUF_LEN, CONTROL_DEVICE_TO_HOST);
    rtos_printf("I2C read started - device control status %d\n", ret);
    return 8;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void rtos_i2c_slave_tx_done_cb(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    rtos_printf("I2C read of %d bytes complete\n", len);
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd_on_tile(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ on tile\n");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }

    return CONTROL_ERROR;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd_on_tile(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control WRITE on tile\n");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command has %d bytes\n", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n");

    return CONTROL_ERROR;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd_off_tile(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ off tile\n");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }

    return CONTROL_ERROR;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd_off_tile(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control WRITE off tile\n");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command has %d bytes\n", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n");

    return CONTROL_ERROR;
}

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    rtos_intertile_start(
            intertile_ctx);

    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);

    control_ret_t dc_ret;

    dc_ret = device_control_start(device_control_ctx,
                                  1, //PORT
                                  configMAX_PRIORITIES-1);

    if (dc_ret != CONTROL_SUCCESS) {
        rtos_printf("Device control failed to start on tile %d\n", THIS_XCORE_TILE);
    } else {
        rtos_printf("Device control started on tile %d\n", THIS_XCORE_TILE);
    }
    xassert(dc_ret == CONTROL_SUCCESS);

    #if ON_TILE(0)
    {
        rtos_printf("Starting GPIO driver\n");
        rtos_gpio_start(gpio_ctx);

        rtos_printf("Starting I2C slave driver\n");
        rtos_i2c_slave_start(i2c_slave_ctx,
                             device_control_ctx,
                             rtos_i2c_slave_start_cb,
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

#if ON_TILE(0)
{
    control_resid_t resources[] = {0x3, 0x6, 0x9};

    device_control_servicer_t servicer_ctx;
    rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);
    dc_ret = device_control_servicer_register(&servicer_ctx,
                                              device_control_ctx,
                                              resources,
                                              sizeof(resources));
    rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

    for (;;) {
        device_control_servicer_cmd_recv(&servicer_ctx, read_cmd_on_tile, write_cmd_on_tile, NULL, RTOS_OSAL_WAIT_FOREVER);
    }
}
#endif
#if ON_TILE(1)
{
    control_resid_t resources[] = {0x33, 0x66, 0x99};

    device_control_servicer_t servicer_ctx;
    rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);
    dc_ret = device_control_servicer_register(&servicer_ctx,
                                              device_control_ctx,
                                              resources,
                                              sizeof(resources));
    rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

    for (;;) {
        device_control_servicer_cmd_recv(&servicer_ctx, read_cmd_off_tile, write_cmd_off_tile, NULL, RTOS_OSAL_WAIT_FOREVER);
    }
}
#endif

    vTaskDelete(NULL);
}


void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{

    (void) c0;
    board_tile0_init(c1, intertile_ctx, i2c_slave_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

    device_control_init(device_control_ctx,
            DEVICE_CONTROL_HOST_MODE,
            &intertile_ctx,
            1);

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

    device_control_init(device_control_ctx,
            DEVICE_CONTROL_CLIENT_MODE,
            &intertile_ctx,
            1);

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
