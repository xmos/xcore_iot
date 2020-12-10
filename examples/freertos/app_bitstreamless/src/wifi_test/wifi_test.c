// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>

#include "FreeRTOS.h"
#include "task.h"

#include "wifi_test.h"

typedef struct {
    rtos_spi_master_device_t *wifi_device_ctx;
    rtos_gpio_t *gpio_ctx;
} wifi_test_devs_t;

static void wifi_test_task(wifi_test_devs_t *wifi_devs)
{
    const rtos_gpio_port_id_t wifi_wup_rst_port = rtos_gpio_port(WIFI_WUP_RST_N);

    rtos_spi_master_device_t *wifi_device_ctx = wifi_devs->wifi_device_ctx;
    rtos_gpio_t *gpio_ctx = wifi_devs->gpio_ctx;

    rtos_gpio_port_enable(gpio_ctx, wifi_wup_rst_port);
    rtos_gpio_port_out(gpio_ctx, wifi_wup_rst_port, 3); /* set bits 0 (WUP) and 1 (RST) high. */

    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t config_read_cmd[2] = {0x80, 0x02}; /* read 2 16-bit words from address 0 */
    uint8_t config_reg[4];


    for (int i = 0; i < 2; i++) {
        rtos_spi_master_transaction_start(wifi_device_ctx);
        rtos_spi_master_transfer(wifi_device_ctx, config_read_cmd, NULL, 2);
        rtos_spi_master_transfer(wifi_device_ctx, NULL, config_reg, 4);
        rtos_spi_master_transaction_end(wifi_device_ctx);
        rtos_printf("Config reg: 0x%02x%02x%02x%02x\n", config_reg[2], config_reg[3], config_reg[0], config_reg[1]);
    }

    vTaskDelete(NULL);
}

void wifi_test_start(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx)
{
    static wifi_test_devs_t wifi_devs;

    wifi_devs.wifi_device_ctx = wifi_device_ctx;
    wifi_devs.gpio_ctx = gpio_ctx;

    xTaskCreate((TaskFunction_t) wifi_test_task,
                "button_deferred_callback",
                RTOS_THREAD_STACK_SIZE(wifi_test_task),
                &wifi_devs,
                configMAX_PRIORITIES/2,
                NULL);
}
