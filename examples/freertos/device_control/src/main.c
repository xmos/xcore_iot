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
#define I2C_DEVICE_CONTROL 1

#define GPIO_TILE_NO 0
#define I2C_TILE_NO 0

static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;

#if USB_DEVICE_CONTROL
#if ON_TILE(USB_TILE_NO)
static device_control_t device_control_usb_ctx_s;
#else
static device_control_client_t device_control_usb_ctx_s;
#endif
static device_control_t *device_control_usb_ctx = (device_control_t *) &device_control_usb_ctx_s;
#endif

#if I2C_DEVICE_CONTROL
#if ON_TILE(I2C_TILE_NO)
static device_control_t device_control_i2c_ctx_s;
#else
static device_control_client_t device_control_i2c_ctx_s;
#endif
static device_control_t *device_control_i2c_ctx = (device_control_t *) &device_control_i2c_ctx_s;
#endif

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

#define DEVICE_CONTROL_USB_PORT 1
#define DEVICE_CONTROL_USB_CLIENT_PRIORITY  (configMAX_PRIORITIES-1)

#define DEVICE_CONTROL_I2C_PORT 2
#define DEVICE_CONTROL_I2C_CLIENT_PRIORITY  (configMAX_PRIORITIES-1)


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
void usb_device_control_set_ctx(device_control_t *ctx,
                                size_t servicer_count);
#endif

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }
    rtos_printf("Bytes to be sent are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control WRITE\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command has %d bytes\n\t", payload_len);
    rtos_printf("Bytes received are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

void vApplicationDaemonTaskStartup(void *arg)
{
    device_control_t *device_control_ctxs[] = {
        #if USB_DEVICE_CONTROL
            device_control_usb_ctx,
        #endif
        #if I2C_DEVICE_CONTROL
            device_control_i2c_ctx,
        #endif
    };

    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    rtos_intertile_start(
            intertile_ctx);

    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);

    #if ON_TILE(GPIO_TILE_NO)
    {
        rtos_printf("Starting GPIO driver\n");
        rtos_gpio_start(gpio_ctx);
    }
    #endif

    control_ret_t dc_ret = CONTROL_SUCCESS;

    #if USB_DEVICE_CONTROL
    {
        if (dc_ret == CONTROL_SUCCESS) {
            dc_ret = device_control_start(device_control_usb_ctx,
                                          DEVICE_CONTROL_USB_PORT,
                                          DEVICE_CONTROL_USB_CLIENT_PRIORITY);
        }
    }
    #endif
    #if I2C_DEVICE_CONTROL
    {
        if (dc_ret == CONTROL_SUCCESS) {
            dc_ret = device_control_start(device_control_i2c_ctx,
                                          DEVICE_CONTROL_I2C_PORT,
                                          DEVICE_CONTROL_I2C_CLIENT_PRIORITY);
        }
    }
    #endif

    if (dc_ret == CONTROL_SUCCESS) {
        rtos_printf("Device control started on tile %d\n", THIS_XCORE_TILE);
    } else {
        rtos_printf("Device control failed to start on tile %d\n", THIS_XCORE_TILE);
    }
    xassert(dc_ret == CONTROL_SUCCESS);

    #if I2C_DEVICE_CONTROL && ON_TILE(I2C_TILE_NO)
    {
        rtos_printf("Starting I2C slave driver\n");
        rtos_i2c_slave_start(i2c_slave_ctx,
                             device_control_i2c_ctx,
                             (rtos_i2c_slave_start_cb_t) i2c_dev_ctrl_start_cb,
                             (rtos_i2c_slave_rx_cb_t) i2c_dev_ctrl_rx_cb,
                             (rtos_i2c_slave_tx_start_cb_t) i2c_dev_ctrl_tx_start_cb,
                             (rtos_i2c_slave_tx_done_cb_t) NULL,
                             0,
                             configMAX_PRIORITIES / 2);
    }
    #endif

    #if USB_DEVICE_CONTROL && ON_TILE(USB_TILE_NO)
    {
        usb_device_control_set_ctx(device_control_usb_ctx, 2);
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
                                                  device_control_ctxs,
                                                  sizeof(device_control_ctxs) / sizeof(device_control_ctxs[0]),
                                                  resources,
                                                  sizeof(resources));
        rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

        for (;;) {
            device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
        }
    }
    #endif
    #if ON_TILE(1)
    {
        control_resid_t resources[] = {0x33, 0x66, 0x99};

        device_control_servicer_t servicer_ctx;
        rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);
        dc_ret = device_control_servicer_register(&servicer_ctx,
                                                  device_control_ctxs,
                                                  sizeof(device_control_ctxs) / sizeof(device_control_ctxs[0]),
                                                  resources,
                                                  sizeof(resources));
        rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

        for (;;) {
            device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
        }
    }
    #endif

    vTaskDelete(NULL);
}

static void tile_common_init(void)
{
    #if I2C_DEVICE_CONTROL
    {
        device_control_init(device_control_i2c_ctx,
                            THIS_XCORE_TILE == I2C_TILE_NO ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                            &intertile_ctx,
                            1);
    }
    #endif

    #if USB_DEVICE_CONTROL
    {
        device_control_init(device_control_usb_ctx,
                            THIS_XCORE_TILE == USB_TILE_NO ? DEVICE_CONTROL_HOST_MODE : DEVICE_CONTROL_CLIENT_MODE,
                            &intertile_ctx,
                            1);

        #if ON_TILE(USB_TILE_NO)
        {
            usb_manager_init();
        }
        #endif
    }
    #endif

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                configMAX_PRIORITIES-1,
                NULL);
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{

    (void) c0;
    board_tile0_init(c1, intertile_ctx, i2c_slave_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

    tile_common_init();

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

    tile_common_init();

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
