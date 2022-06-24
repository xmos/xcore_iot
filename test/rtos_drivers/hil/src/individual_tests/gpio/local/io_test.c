// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_gpio.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/gpio/gpio_test.h"

static const char* test_name = "io_test";

#define local_printf( FMT, ... )    gpio_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define GPIO_TILE 1

#if ON_TILE(GPIO_TILE)
RTOS_GPIO_ISR_CALLBACK_ATTR
static void input_cb(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t xYieldRequired = pdFALSE;

    local_printf("input_cb ISR triggered");
    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &xYieldRequired);

    local_printf("Disable interrupt on 0x%x", port_id);
    rtos_gpio_interrupt_disable(ctx, port_id);
    portYIELD_FROM_ISR(xYieldRequired);
}

static void output_thread(rtos_gpio_t *ctx)
{
    const rtos_gpio_port_id_t p_test_output = rtos_gpio_port(OUTPUT_PORT);

    // Gives the main test some time to prepare
    vTaskDelay(pdMS_TO_TICKS(1));

    local_printf("Output 1");
    uint32_t val = rtos_gpio_port_in(ctx, p_test_output);
    rtos_gpio_port_out(ctx, p_test_output, (1 << OUTPUT_PORT_PIN_OFFSET) | val);
    vTaskSuspend(NULL);
    while(1) {;}
}
#endif

GPIO_MAIN_TEST_ATTR
static int main_test(gpio_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(GPIO_TILE)
    {
        const rtos_gpio_port_id_t p_test_input = rtos_gpio_port(INPUT_PORT);
        const rtos_gpio_port_id_t p_test_output = rtos_gpio_port(OUTPUT_PORT);

        uint32_t status = 0;
        uint32_t val = 0;

        local_printf("Set value to 1");
        val = rtos_gpio_port_in(ctx->gpio_ctx, p_test_output);
        rtos_gpio_port_out(ctx->gpio_ctx, p_test_output, (1 << OUTPUT_PORT_PIN_OFFSET) | val);

        val = rtos_gpio_port_in(ctx->gpio_ctx, p_test_input);
        val &= (1 << INPUT_PORT_PIN_OFFSET);

        if (val != 1)
        {
            local_printf("Simple read and write failed.  Got %u expected %u", val, 1);
            return -1;
        } else {
            local_printf("Simple read and write passed.  Got %u expected %u", val, 1);
        }

        local_printf("Set value to 0");
        val = rtos_gpio_port_in(ctx->gpio_ctx, p_test_output);
        rtos_gpio_port_out(ctx->gpio_ctx, p_test_output, ~(1 << OUTPUT_PORT_PIN_OFFSET) & val);

        val = rtos_gpio_port_in(ctx->gpio_ctx, p_test_input);
        val &= (1 << INPUT_PORT_PIN_OFFSET);

        if (val != 0)
        {
            local_printf("Simple read and write failed.  Got %u expected %u", val, 0);
            return -1;
        } else {
            local_printf("Simple read and write passed.  Got %u expected %u", val, 0);
        }

        local_printf("Enable ISR on input");
        rtos_gpio_isr_callback_set(ctx->gpio_ctx, p_test_input, input_cb, xTaskGetCurrentTaskHandle());
        rtos_gpio_interrupt_enable(ctx->gpio_ctx, p_test_input);

        local_printf("Waiting for timeout notify");
        BaseType_t ret = xTaskNotifyWait( 0x00000000UL,    /* Don't clear notification bits on entry */
                                          0xFFFFFFFFUL,    /* Reset full notification value on exit */
                                          &status,         /* Pass out notification value into status */
                                          1 );             /* Wait 1 tick */
        if (ret != pdFALSE)
        {
            local_printf("Timeout notify did not time out");
            return -1;
        }

        TaskHandle_t output_handle;

        xTaskCreate((TaskFunction_t)output_thread,
                    "gpio_output",
                    RTOS_THREAD_STACK_SIZE(output_thread),
                    ctx->gpio_ctx,
                    configMAX_PRIORITIES-1,
                    &output_handle);

        local_printf("Waiting for notify from ISR");
        ret = xTaskNotifyWait( 0x00000000UL,         /* Don't clear notification bits on entry */
                               0xFFFFFFFFUL,         /* Reset full notification value on exit */
                               &status,              /* Pass out notification value into status */
                               pdMS_TO_TICKS(10) );  /* Wait 10 ms for notification */
        if (ret == pdFALSE)
        {
            local_printf("ISR should have triggered by now");
            return -1;
        }

        val = rtos_gpio_port_in(ctx->gpio_ctx, p_test_input);
        val &= (1 << INPUT_PORT_PIN_OFFSET);

        if (val != 1)
        {
            local_printf("Read after ISR trigger failed.  Got %u expected %u", val, 1);
            return -1;
        } else {
            local_printf("Read after ISR trigger passed.  Got %u expected %u", val, 1);
        }

        local_printf("Delete output thread");
        vTaskDelete(output_handle);
    }
    #endif

    local_printf("Done");
    return 0;
}

void register_io_test(gpio_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
