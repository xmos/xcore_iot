// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>

#include "FreeRTOS.h"

#include "platform/app_pll_ctrl.h"
#include "gpio_ctrl/gpi_ctrl.h"

__attribute__((weak))
void gpio_gpi_toggled_cb(uint32_t gpio_val)
{
    // rtos_printf("A GPI toggled\n");
}

RTOS_GPIO_ISR_CALLBACK_ATTR
static void gpio_callback(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t yield_required = pdFALSE;

    value = (~value) & GPIO_BITMASK;

    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &yield_required);

    portYIELD_FROM_ISR(yield_required);
}

static void gpio_handler(rtos_gpio_t *gpio_ctx)
{
    uint32_t value;
    uint32_t gpio_val;

    const rtos_gpio_port_id_t gpio_port = rtos_gpio_port(GPIO_PORT);

    rtos_gpio_port_enable(gpio_ctx, gpio_port);

    rtos_gpio_isr_callback_set(gpio_ctx, gpio_port, gpio_callback, xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx, gpio_port);

    for (;;) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                &value,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */

        gpio_val = rtos_gpio_port_in(gpio_ctx, gpio_port);
        gpio_gpi_toggled_cb(gpio_val);
    }
}

void gpio_gpi_init(rtos_gpio_t *gpio_ctx)
{
    if (GPIO_PORT != 0) {
        xTaskCreate((TaskFunction_t) gpio_handler,
                    "gpio_handler",
                    RTOS_THREAD_STACK_SIZE(gpio_handler),
                    gpio_ctx,
                    configMAX_PRIORITIES-2,
                    NULL);
    }
}
