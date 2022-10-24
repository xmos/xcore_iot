// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"

#include "rtos_printf.h"

#include "platform/platform_init.h"
#include "platform/driver_instances.h"

/*
 * Constants / Macros
 */

typedef enum ProcessState {
    STATE_IDLE,
    STATE_SETUP,
    STATE_RUN,
    STATE_TIMEOUT
} ProcessState_t;

#define STRINGIFY(str)                    #str

#define REF_TMR_TICKS_PER_MS              (PLATFORM_REFERENCE_HZ / 1000)
#define REF_TMR_TICKS_FROM_MS(x)          ((x) * REF_TMR_TICKS_PER_MS)

#define PROCESS_TIMEOUT_DELAY_MS          1000 // How long to stay in the timeout state
#define PROCESS_IDLE_DELAY_MS             500  // How long to stay in the timeout state
#define PROCESS_RUN_DELAY_MS              1    // The time between each "run" iteration/output value
#define PROCESS_RUN_ITERATIONS            2048 // (PROCESS_RUN_DELAY_MS * PROCESS_RUN_ITERATIONS) == min duration of "run" state.
#define PROCESS_WAVEFORM(x)               (uint32_t)(1000 * sin(2 * M_PI * (x) / (PROCESS_RUN_ITERATIONS / 4)) + 1000)
#define PROCESS_TIMEOUT_TICKS             REF_TMR_TICKS_FROM_MS(20)

#define NUM_SUBPROCESS_TASKS              8
#define SUBPROCESS_DELAY_TIME_MS          5   /* Represents the time a subprocess is running and
                                               * effectively holding onto the mutex */
#define SUBPROCESS_TIMER_MS               100 /* Time delay from when main process runs to when the
                                               * first subprocess is notified. This time should not
                                               * exceed (PROCESS_RUN_ITERATIONS * PROCESS_RUN_DELAY_MS) */

#define TASK_NOTIF_MASK_RUN_TASK          0x00010000
#define TASK_NOTIF_MASK_HBEAT_TIMER       0x00000010
#define TASK_NOTIF_MASK_TIMEOUT           0x00000020
#define TASK_NOTIF_MASK_BTN_EVENT         0x00000004
#define TASK_NOTIF_MASK_BTN_MASK          0x00000003

#define BUTTON_DEBOUNCE_MS                100
#define BUTTON_DEBOUNCE_TICKS             REF_TMR_TICKS_FROM_MS(BUTTON_DEBOUNCE_MS)
#define BUTTON0_PRESSED(notif_val)        ((notif_val >> 0) & 0x01)
#define BUTTON1_PRESSED(notif_val)        ((notif_val >> 1) & 0x01)

#define LED_HBEAT_TIMER_MS                500

#define LED_MASK_BTN0                     0x01
#define LED_MASK_BTN1                     0x02
#define LED_MASK_TMO                      0x04
#define LED_MASK_HBEAT                    0x08

#define LED_ASSERT(buf, mask)             buf |= mask
#define LED_DEASSERT(buf, mask)           buf &= ~mask
#define LED_TOGGLE(buf, mask)             buf ^= mask
#define LED_UPDATE(cond, buf, mask)       \
    do {                                  \
        if (cond) {                       \
            LED_ASSERT(buf, mask);        \
        } else {                          \
            LED_DEASSERT(buf, mask);      \
        }                                 \
    } while (0)

/*
 * If another trace implementation is being used, define
 * TRC_CFG_ENTRY_SYMBOL_MAX_LENGTH to give the buffer used to generate the
 * subprocess task names a define size
 */
#ifndef TRC_CFG_ENTRY_SYMBOL_MAX_LENGTH
#define TRC_CFG_ENTRY_SYMBOL_MAX_LENGTH 32
#endif

/*
 * Private Globals
 */

static const UBaseType_t base_task_priority =
        configMAX_PRIORITIES - NUM_SUBPROCESS_TASKS - 2;

#if ON_TILE(0)

#if (TRC_USE_TRACEALYZER_RECORDER == 1)
static ProcessState_t process_state_last = STATE_TIMEOUT;
static const char *states[] = { "Idle", "Setup", "Run", "Timeout" };
#endif

static TaskHandle_t ctx_gpio_task = NULL;
static TaskHandle_t ctx_process_task = NULL;
static TaskHandle_t ctx_subprocess_tasks[NUM_SUBPROCESS_TASKS] = { NULL };
static SemaphoreHandle_t resource_mutex;
static uint8_t num_requested_subprocesses = 0;
static uint8_t num_subprocesses = 0;
static ProcessState_t process_state = STATE_IDLE;

#if (TRC_USE_TRACEALYZER_RECORDER == 1)
static TraceStringHandle_t trc_main_state;
static TraceStringHandle_t trc_active_subprocesses;
static TraceStringHandle_t trc_run_iteration;
static TraceStringHandle_t trc_run_delta;
static TraceStringHandle_t trc_output;
#endif

#endif /* ON_TILE(0) */

/*
 * Forward Declarations
 */

#if ON_TILE(0)
static inline uint32_t get_abs_delta(uint32_t current, uint32_t last);

RTOS_GPIO_ISR_CALLBACK_ATTR
static void button_callback(rtos_gpio_t *ctx, void *app_data,
                            rtos_gpio_port_id_t port_id, uint32_t value);
static void on_btn0_event(uint8_t *num_subprocesses);
static void on_btn1_event(uint8_t *num_subprocesses);
static uint8_t is_btn_stable(uint32_t curr_btn_ticks, uint32_t *last_btn_ticks);

static void hbeat_tmr_callback(TimerHandle_t pxTimer);
static void subprocess_tmr_callback(TimerHandle_t pxTimer);
static void spin_ref_tmr_ticks(uint32_t ticks);

static void process_task(void *arg);
static void subprocess_task(void *arg);
static void gpio_task(void *arg);

#endif /* ON_TILE(0) */

static void hello_task(void *arg);
static void tile_common_init(chanend_t c);

/*
 * Private Definitions
 */

#if ON_TILE(0)

static inline uint32_t get_abs_delta(uint32_t current, uint32_t last)
{
    return (current >= last) ? (current - last) : (UINT32_MAX - last + current);
}

RTOS_GPIO_ISR_CALLBACK_ATTR
static void button_callback(rtos_gpio_t *ctx, void *app_data,
                            rtos_gpio_port_id_t port_id, uint32_t value)
{
    const uint32_t btn0Mask = 0x01;
    const uint32_t btn1Mask = 0x02;
    TaskHandle_t task = app_data;
    BaseType_t xYieldRequired = pdFALSE;

    // Convert active-low logic to active-high
    value = (~value) & (btn0Mask | btn1Mask);

    xTaskNotifyFromISR(task, (value | TASK_NOTIF_MASK_BTN_EVENT),
                       eSetValueWithOverwrite, &xYieldRequired);
    portYIELD_FROM_ISR(xYieldRequired);
}

static uint8_t is_btn_stable(uint32_t curr_btn_ticks, uint32_t *last_btn_ticks)
{
    uint32_t delta_ticks = get_abs_delta(curr_btn_ticks, *last_btn_ticks);
    uint8_t btn_stable = (delta_ticks >= BUTTON_DEBOUNCE_TICKS);
    *last_btn_ticks = curr_btn_ticks;
    return btn_stable;
}

static void on_btn0_event(uint8_t *num_subprocesses)
{
    if (*num_subprocesses > 0) {
        (*num_subprocesses)--;
#if ((TRC_USE_TRACEALYZER_RECORDER == 1) && (TRC_CFG_INCLUDE_USER_EVENTS == 1))
        xTracePrintF(trc_active_subprocesses, "%d", *num_subprocesses);
#endif
        rtos_printf("Num active subprocesses: %d\n", *num_subprocesses);
    }
}

static void on_btn1_event(uint8_t *num_subprocesses)
{
    if (*num_subprocesses < NUM_SUBPROCESS_TASKS) {
        (*num_subprocesses)++;
#if ((TRC_USE_TRACEALYZER_RECORDER == 1) && (TRC_CFG_INCLUDE_USER_EVENTS == 1))
        xTracePrintF(trc_active_subprocesses, "%d", *num_subprocesses);
#endif
        rtos_printf("Num active subprocesses: %d\n", *num_subprocesses);
    }
}

static void hbeat_tmr_callback(TimerHandle_t pxTimer)
{
    xTaskNotify(ctx_gpio_task, TASK_NOTIF_MASK_HBEAT_TIMER, eSetBits);
}

static void subprocess_tmr_callback(TimerHandle_t pxTimer)
{
    if (num_subprocesses > 0) {
        xTaskNotify(ctx_subprocess_tasks[0], TASK_NOTIF_MASK_RUN_TASK,
                    eSetBits);
    }
}

static void spin_ref_tmr_ticks(uint32_t ticks)
{
    uint32_t current_ticks = get_reference_time();
    uint32_t stop_ticks = current_ticks + ticks;
    uint8_t rollover_condition = (stop_ticks < current_ticks);

    while (1) {
        if (rollover_condition && (get_reference_time() < stop_ticks)) {
            rollover_condition = 0;
        } else if (get_reference_time() >= stop_ticks) {
            break;
        }
    }
}

static void process_task(void *arg)
{
    /*
     * This value is used to limit how frequently to produce user event traces.
     * Creating too many events may lead to data loss due to bandwidth
     * limitations. This may be observed by closely inspecting the sinusoidal
     * (or run iteration) waveform being generated in this example. In such
     * cases, a sample will be held onto for a period of time and then there
     * will be an "unexpected" phase jump.
     */
    const uint8_t trace_subsample_count = 8;

    uint32_t current_run_ticks;
    uint32_t last_run_ticks;
    uint32_t delta_ticks;
    uint32_t run_iteration;
    TimerHandle_t tmr_subprocess;

    rtos_printf("Entered main process on core %d\n", portGET_CORE_ID());

    tmr_subprocess = xTimerCreate(STRINGIFY(tmr_subprocess),
                                  pdMS_TO_TICKS(SUBPROCESS_TIMER_MS), pdFALSE,
                                  NULL, subprocess_tmr_callback);

    while (1) {
#if (TRC_USE_TRACEALYZER_RECORDER == 1)
        if (process_state != process_state_last) {
            process_state_last = process_state;
            xTracePrint(trc_main_state, states[process_state]);
        }
#endif

        switch (process_state) {
        default:
        case STATE_IDLE:
            process_state = STATE_SETUP;
            vTaskDelay(pdMS_TO_TICKS(PROCESS_IDLE_DELAY_MS));
            break;

        case STATE_SETUP:
            num_subprocesses = num_requested_subprocesses;
            /*
             * Start a timer which will fire off a set of subtasks which will
             * preempt this task during the "run" state. This will potentially
             * cause a timing issue that the state logic will detect which can
             * then be observed/inspected using Percepio's Tracealyzer.
             */
            xTimerStart(tmr_subprocess, 0);
            process_state = STATE_RUN;
            last_run_ticks = get_reference_time();
            run_iteration = 0;
            break;

        case STATE_RUN:
            while (process_state == STATE_RUN) {
                current_run_ticks = get_reference_time();
                delta_ticks = get_abs_delta(current_run_ticks, last_run_ticks);
                last_run_ticks = current_run_ticks;

#if ((TRC_USE_TRACEALYZER_RECORDER == 1) && (TRC_CFG_INCLUDE_USER_EVENTS == 1))
                if ((run_iteration % trace_subsample_count) == 1) {
                    xTracePrintF(trc_run_iteration, "%d", run_iteration);
                }
#endif

                if (delta_ticks >= PROCESS_TIMEOUT_TICKS) {
                    process_state = STATE_TIMEOUT;
                    break;
                }

#if ((TRC_USE_TRACEALYZER_RECORDER == 1) && (TRC_CFG_INCLUDE_USER_EVENTS == 1))
                uint32_t output = PROCESS_WAVEFORM(run_iteration);
                if ((run_iteration % trace_subsample_count) == 1) {
                    xTracePrintF(trc_output, "%d", output);
                }
#endif
                /*
                 * In this example, the main process and subprocesses use
                 * "spin_ref_tmr_ticks" as a means to convey a shared resource
                 * which takes some time to utilize.
                 */
                xSemaphoreTake(resource_mutex, portMAX_DELAY);
                spin_ref_tmr_ticks(REF_TMR_TICKS_FROM_MS(PROCESS_RUN_DELAY_MS));
                xSemaphoreGive(resource_mutex);

                run_iteration++;
                if (run_iteration >= PROCESS_RUN_ITERATIONS) {
                    process_state = STATE_IDLE;
                }
            }

            break;

        case STATE_TIMEOUT:
            rtos_printf("Timeout!\n");
            xTaskNotify(ctx_gpio_task, TASK_NOTIF_MASK_TIMEOUT, eSetBits);
            vTaskDelay(pdMS_TO_TICKS(PROCESS_TIMEOUT_DELAY_MS));
            process_state = STATE_SETUP;
            break;
        }
    }
}

static void subprocess_task(void *arg)
{
    const uint32_t bits_to_clear_on_entry = 0x00000000UL;
    const uint32_t bits_to_clear_on_exit = 0xFFFFFFFFUL;
    uint32_t notif_value;
    uint8_t inst = ((uint32_t)arg - (uint32_t)ctx_subprocess_tasks) /
                   sizeof(TaskHandle_t);
    uint8_t next_inst = inst + 1;

    rtos_printf("Entered subprocess task (%d) on core %d\n", inst,
                portGET_CORE_ID());

    while (1) {
        xTaskNotifyWait(bits_to_clear_on_entry, bits_to_clear_on_exit,
                        &notif_value, portMAX_DELAY);

        if (notif_value & TASK_NOTIF_MASK_RUN_TASK) {
            xSemaphoreTake(resource_mutex, portMAX_DELAY);
            spin_ref_tmr_ticks(REF_TMR_TICKS_FROM_MS(SUBPROCESS_DELAY_TIME_MS));

            /*
             * Notify before giving back the mutex to allow the higher priority
             * task to queue up for the notification event. This allows the
             * subprocesses to run sequentially, while keeping the main process
             * task in a blocked state.
             */
            if (next_inst < num_subprocesses) {
                xTaskNotify(ctx_subprocess_tasks[next_inst],
                            TASK_NOTIF_MASK_RUN_TASK, eSetBits);
            }

            xSemaphoreGive(resource_mutex);
        }
    }
}

static void gpio_task(void *arg)
{
    const uint32_t bits_to_clear_on_entry = 0x00000000UL;
    const uint32_t bits_to_clear_on_exit = 0xFFFFFFFFUL;
    uint32_t led_val = 0;
    uint32_t notif_value;
    TimerHandle_t tmr_hbeat;
    rtos_gpio_port_id_t p_leds = rtos_gpio_port(PORT_LEDS);
    rtos_gpio_port_id_t p_btns = rtos_gpio_port(PORT_BUTTONS);
    uint32_t btn0_debounce_tick_time = get_reference_time();
    uint32_t btn1_debounce_tick_time = btn0_debounce_tick_time;

    rtos_gpio_port_enable(gpio_ctx_t0, p_leds);
    rtos_gpio_port_enable(gpio_ctx_t0, p_btns);
    rtos_gpio_isr_callback_set(gpio_ctx_t0, p_btns, button_callback,
                               xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx_t0, p_btns);

    tmr_hbeat = xTimerCreate(STRINGIFY(tmr_hbeat),
                             pdMS_TO_TICKS(LED_HBEAT_TIMER_MS), pdTRUE, NULL,
                             hbeat_tmr_callback);

    xTimerStart(tmr_hbeat, 0);

    rtos_printf("Entered gpio task on core %d\n", portGET_CORE_ID());

    while (1) {
        xTaskNotifyWait(bits_to_clear_on_entry, bits_to_clear_on_exit,
                        &notif_value, portMAX_DELAY);

        if (notif_value & TASK_NOTIF_MASK_HBEAT_TIMER) {
            LED_TOGGLE(led_val, LED_MASK_HBEAT);
        }

        if (notif_value & TASK_NOTIF_MASK_TIMEOUT) {
            LED_TOGGLE(led_val, LED_MASK_TMO);
        }

        if (notif_value & TASK_NOTIF_MASK_BTN_EVENT) {
            uint32_t current_ref_ticks = get_reference_time();

            if (BUTTON0_PRESSED(notif_value) &&
                is_btn_stable(current_ref_ticks, &btn0_debounce_tick_time)) {
                LED_ASSERT(led_val, LED_MASK_BTN0);
                on_btn0_event(&num_requested_subprocesses);
            } else {
                LED_DEASSERT(led_val, LED_MASK_BTN0);
                btn0_debounce_tick_time = current_ref_ticks;
            }

            if (BUTTON1_PRESSED(notif_value) &&
                is_btn_stable(current_ref_ticks, &btn1_debounce_tick_time)) {
                LED_ASSERT(led_val, LED_MASK_BTN1);
                on_btn1_event(&num_requested_subprocesses);
            } else {
                LED_DEASSERT(led_val, LED_MASK_BTN1);
                btn1_debounce_tick_time = current_ref_ticks;
            }
        }

        rtos_gpio_port_out(gpio_ctx_t0, p_leds, led_val);
    }
}

#endif /* ON_TILE(0) */

static void hello_task(void *arg)
{
    rtos_printf("Hello task running from tile %d on core %d\n", THIS_XCORE_TILE,
                portGET_CORE_ID());

    for (;;) {
        rtos_printf("Hello from tile %d\n", THIS_XCORE_TILE);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);

#if ON_TILE(0)

    resource_mutex = xSemaphoreCreateMutex();
#if (TRC_USE_TRACEALYZER_RECORDER == 1)
    vTraceSetSemaphoreName(resource_mutex, "Shared Resource");
    trc_main_state = xTraceRegisterString("Main State");
    trc_active_subprocesses = xTraceRegisterString("Active Subprocesses");
    trc_run_iteration = xTraceRegisterString("Run Iteration");
    trc_run_delta = xTraceRegisterString("Run Delta");
    trc_output = xTraceRegisterString("Output Signal");
#endif

    xTaskCreate((TaskFunction_t)gpio_task, STRINGIFY(gpio_task),
                RTOS_THREAD_STACK_SIZE(gpio_task), NULL,
                configMAX_PRIORITIES - 1, &ctx_gpio_task);
    xTaskCreate((TaskFunction_t)process_task, STRINGIFY(process_task),
                RTOS_THREAD_STACK_SIZE(process_task), NULL, base_task_priority,
                &ctx_process_task);

    /* Spawn subprocess tasks at one level higher priority than the previous */
    for (int i = 0; i < NUM_SUBPROCESS_TASKS; i++) {
        char task_name[TRC_CFG_ENTRY_SYMBOL_MAX_LENGTH];

        snprintf(task_name, sizeof(task_name), "%s_%02d",
                 STRINGIFY(subprocess_task), i);
        xTaskCreate((TaskFunction_t)subprocess_task, task_name,
                    RTOS_THREAD_STACK_SIZE(subprocess_task),
                    &ctx_subprocess_tasks[i], base_task_priority + i + 1,
                    &ctx_subprocess_tasks[i]);
    }

#endif /* ON_TILE(0) */

    xTaskCreate((TaskFunction_t)hello_task, STRINGIFY(hello_task),
                RTOS_THREAD_STACK_SIZE(hello_task), NULL, base_task_priority,
                NULL);

    rtos_printf("Start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

/*
 * Public Definitions
 */

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c2;
    (void)c3;
#if (USE_TRACE_MODE == TRACE_MODE_TRACEALYZER_STREAMING)
    xTraceInitialize();
    xTraceEnable(TRC_START);
#endif

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c1;
    (void)c2;
    (void)c3;

    tile_common_init(c0);
}
#endif