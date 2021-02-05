// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <platform.h>

#include "gpio_test.h"

RTOS_GPIO_ISR_CALLBACK_ATTR
static void button_callback(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t yield_required = pdFALSE;

    value = (~value) & 0x3;

    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &yield_required);

    portYIELD_FROM_ISR(yield_required);
}

static void button_deferred_callback(rtos_gpio_t *gpio_ctx)
{
    const rtos_gpio_port_id_t button_port = rtos_gpio_port(PORT_BUTTONS);
    const rtos_gpio_port_id_t led_port = rtos_gpio_port(PORT_LEDS);

    rtos_gpio_port_enable(gpio_ctx, led_port);
    rtos_gpio_port_enable(gpio_ctx, button_port);

    rtos_gpio_isr_callback_set(gpio_ctx, button_port, button_callback, xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx, button_port);

    for (;;) {
        uint32_t value;

        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                &value,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */

        rtos_printf("Button value from ISR is %d\n", value);
        rtos_printf("Actual button port value is %d\n", rtos_gpio_port_in(gpio_ctx, button_port));

        rtos_gpio_port_out(gpio_ctx, led_port, value);

        if (value == 3) {
            rtos_printf("Disabling the button interrupt for 5 seconds\n");
            rtos_gpio_interrupt_disable(gpio_ctx, button_port);

            vTaskDelay(pdMS_TO_TICKS(5000));

            rtos_printf("Re-enabling the button interrupt\n");
            rtos_gpio_interrupt_enable(gpio_ctx, button_port);
        }
    }
}

void gpio_test_start(rtos_gpio_t *gpio_ctx)
{

    xTaskCreate((TaskFunction_t) button_deferred_callback,
                "button_deferred_callback",
                RTOS_THREAD_STACK_SIZE(button_deferred_callback),
                gpio_ctx,
                configMAX_PRIORITIES-2,
                NULL);
}
