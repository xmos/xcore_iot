// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

#include "FreeRTOS.h"
#include "task.h"

#include "usb_support.h"

#include "device_control.h"

#include "rtos_printf.h"

#include "board_init.h"

#define USB_DEVICE_CONTROL 1
#define I2C_DEVICE_CONTROL 0

#if (USB_DEVICE_CONTROL + I2C_DEVICE_CONTROL) != 1
#error Must define exactly one device control transport
#endif

#define DEVICE_CONTROL_HOST_TILE (I2C_DEVICE_CONTROL ? 0 : (USB_DEVICE_CONTROL ? USB_TILE_NO : -1))

static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;

#if ON_TILE(DEVICE_CONTROL_HOST_TILE)
static device_control_t device_control_ctx_s;
#else
static device_control_client_t device_control_ctx_s;
#endif

static rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static device_control_t *device_control_ctx = (device_control_t *) &device_control_ctx_s;

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

#define GPIO_RPC_PORT 0
#define GPIO_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES/2)


#if I2C_DEVICE_CONTROL
void i2c_dev_ctrl_start_cb(rtos_i2c_slave_t *ctx,
                           device_control_t *device_control_ctx);

void i2c_dev_ctrl_rx_cb(rtos_i2c_slave_t *ctx,
                        device_control_t *device_control_ctx,
                        uint8_t *data,
                        size_t len);

size_t i2c_dev_ctrl_tx_start_cb(rtos_i2c_slave_t *ctx,
                                device_control_t *device_control_ctx,
                                uint8_t **data);
#endif

#if USB_DEVICE_CONTROL
void usb_device_control_set_ctx(device_control_t *ctx);
#endif

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd_on_tile(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ on tile\n");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }

    return CONTROL_SUCCESS;
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

    return CONTROL_SUCCESS;
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

    return CONTROL_SUCCESS;
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

    return CONTROL_SUCCESS;
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

        #if I2C_DEVICE_CONTROL
        {
            rtos_printf("Starting I2C slave driver\n");
            rtos_i2c_slave_start(i2c_slave_ctx,
                                 device_control_ctx,
                                 (rtos_i2c_slave_start_cb_t) i2c_dev_ctrl_start_cb,
                                 (rtos_i2c_slave_rx_cb_t) i2c_dev_ctrl_rx_cb,
                                 (rtos_i2c_slave_tx_start_cb_t) i2c_dev_ctrl_tx_start_cb,
                                 (rtos_i2c_slave_tx_done_cb_t) NULL,
                                 configMAX_PRIORITIES / 2);
        }
        #endif
    }
    #endif

    #if USB_DEVICE_CONTROL && ON_TILE(USB_TILE_NO)
    {
        usb_device_control_set_ctx(device_control_ctx);
        usb_manager_start(configMAX_PRIORITIES - 1);
    }
    #endif

    chanend_free(other_tile_c);

#if ON_TILE(0)
{
    control_resid_t resources[] = {0x3, 0x6, 0x9};

    device_control_servicer_t servicer_ctx;
    rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);
    dc_ret = device_control_servicer_register(&servicer_ctx,
                                              &device_control_ctx,
                                              1,
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
                                              &device_control_ctx,
                                              1,
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

#if ON_TILE(DEVICE_CONTROL_HOST_TILE)
    rtos_printf("Tile %d is host\n", THIS_XCORE_TILE);
#endif

    device_control_init(device_control_ctx,
                        DEVICE_CONTROL_HOST_TILE == THIS_XCORE_TILE ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                        &intertile_ctx,
                        1);

#if USB_DEVICE_CONTROL && ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif

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

#if ON_TILE(DEVICE_CONTROL_HOST_TILE)
    rtos_printf("Tile %d is host\n", THIS_XCORE_TILE);
#endif

    device_control_init(device_control_ctx,
                        DEVICE_CONTROL_HOST_TILE == 1 ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                        &intertile_ctx,
                        1);

#if USB_DEVICE_CONTROL && ON_TILE(USB_TILE_NO)
    usb_manager_init();
#endif

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
