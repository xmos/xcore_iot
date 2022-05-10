// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "keyword_inference.h"
#include "inference_hmi/inference_hmi.h"
#include "ssd1306.h"
#include "font8x8_basic.h"

void ssd1306_128x32_ascii_to_bitmap(char* str_buf, char* bitmap);

void ssd1306_128x32_clear_bitmap(char* bitmap);
void ssd1306_128x32_add_indicator_frame_to_bitmap(char* bitmap);

#define KEYWORD_DETECT_DEBOUNCE       1
// TODO: change the silence and unknown resets to a wall block based reset
#define KEYWORD_DETECT_UNKNOWN_RESET  20
#define KEYWORD_DETECT_SILENCE_RESET  20

typedef enum inference_state {
    STATE_WAIT_FOR_ANY,
    STATE_WAIT_FOR_ACTION,
    STATE_WAIT_FOR_OBJECT,
} inference_state_t;

rtos_gpio_port_id_t gpo_port = 0;

#if XK_VOICE_L71
#define gpo_setup()     {                                       \
    gpo_port = rtos_gpio_port(PORT_GPO);                        \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);               \
}
#define green_led_on()  {                                       \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<5));  \
}
#define green_led_off()  {                                      \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<5));   \
}
#define red_led_on()  {                                         \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<4));  \
}
#define red_led_off()  {                                        \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<4));   \
}

#elif XCOREAI_EXPLORER
/* LED 0 is "green"
 * LED 1 is "red" */
#define gpo_setup()     {                                      \
    gpo_port = rtos_gpio_port(PORT_LEDS);                      \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);              \
}
#define green_led_on()  {                                      \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<0));  \
}
#define green_led_off()  {                                     \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<0)); \
}
#define red_led_on()  {                                        \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<1));  \
}
#define red_led_off()  {                                       \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<1)); \
}
#endif

__attribute__(( fptrgroup("ssd1306_transport_write") ))
size_t ssd1306_I2C_write(void* app_ctx, void* bus, int address, uint8_t *buf, size_t len) {
    size_t num_bytes_sent = 0;

    rtos_i2c_master_write(
            (rtos_i2c_master_t*)bus,
            (uint8_t)address,
            buf,
            len,
            &num_bytes_sent,
            1);

    return num_bytes_sent;
}

void inference_hmi_task(void *args)
{
    EventGroupHandle_t egrp_intent = (EventGroupHandle_t) args;
    EventBits_t rx_bits = 0;
    EventBits_t last_rx_bits = 0;
    EventBits_t successive_cnt = 0;
    inference_state_t state = STATE_WAIT_FOR_ANY;
    inference_state_t last_state = STATE_WAIT_FOR_ANY;
    uint32_t intent = 0;
    ssd1306_context ssd1306_ctx_s;
    ssd1306_context* ssd1306_ctx = &ssd1306_ctx_s;
    ssd1306_transport ssd1306_transport = {i2c_master_ctx, 0x3C, &ssd1306_I2C_write};
    uint8_t display_buf[512];

    gpo_setup();

    /* Initialize and clear the screen */
    ssd1306_init(
        NULL,
        ssd1306_ctx,
        &ssd1306_transport,
        ssd1306_MDOB128032GV);

    ssd1306_128x32_clear_bitmap(display_buf);
    ssd1306_write(NULL, ssd1306_ctx, display_buf);

    while(1) {
        /* Wait forever for a bit change.  Clear changed bit on exit */
        rx_bits = xEventGroupWaitBits(
                egrp_intent,
                INFERENCE_BIT_ALL,
                pdTRUE,
                pdFALSE,
                portMAX_DELAY);

        /* If there have been KEYWORD_DETECT_DEBOUNCE successive
         * keywords spotted, assume it to be valid and run the state machine */
        if (rx_bits == last_rx_bits) {
            successive_cnt++;
        } else {
            successive_cnt = 0;
        }
        last_rx_bits = rx_bits;

        /* Assume in the unknown case that it is just "filler" words
         * IE: "open the door" -> ACTION FILLER OBJECT */
        if ((rx_bits & INFERENCE_BIT_SPOTTED_UNKNOWN) != 0) {
            if (successive_cnt == KEYWORD_DETECT_UNKNOWN_RESET) {
                intent = 0;
                state = STATE_WAIT_FOR_ANY;
                rtos_printf("Unknown threshold passed.  Reset intent state\n");
                ssd1306_128x32_clear_bitmap(display_buf);
            }
        /* Similarly to "filler" words, we must filter out inter word silence */
        } else if ((rx_bits & INFERENCE_BIT_SPOTTED_SILENCE) != 0) {
           if (successive_cnt == KEYWORD_DETECT_SILENCE_RESET) {
               intent = 0;
               state = STATE_WAIT_FOR_ANY;
               rtos_printf("Silence threshold passed.  Reset intent state\n");
               ssd1306_128x32_clear_bitmap(display_buf);
           }
        } else {
            /* Only run the state machine once per non-unknown keyword */
            if (successive_cnt == KEYWORD_DETECT_DEBOUNCE) {
                last_state = state;
                switch (state) {
                    default:
                    case STATE_WAIT_FOR_ANY:
                        if ( ((rx_bits & INFERENCE_BIT_DISPLAY) != 0)
                          || ((rx_bits & INFERENCE_BIT_CLEAR) != 0) ) {
                              state = STATE_WAIT_FOR_OBJECT;
                              intent |= rx_bits;
                              rtos_printf("Found object wait for action\n");
                              ssd1306_128x32_add_indicator_frame_to_bitmap((char *)display_buf);
                        } else if ( ((rx_bits & INFERENCE_BIT_GREEN) != 0)
                                 || ((rx_bits & INFERENCE_BIT_RED) != 0) ) {
                              state = STATE_WAIT_FOR_ACTION;
                              intent |= rx_bits;
                              rtos_printf("Found action wait for object\n");
                              ssd1306_128x32_add_indicator_frame_to_bitmap((char *)display_buf);
                        }
                        break;
                    case STATE_WAIT_FOR_ACTION:
                        if ( ((rx_bits & INFERENCE_BIT_DISPLAY) != 0)
                          || ((rx_bits & INFERENCE_BIT_CLEAR) != 0) ) {
                          intent |= rx_bits;
                        } else {
                            rtos_printf("Action with no object.  Reset intent state\n");
                            ssd1306_128x32_clear_bitmap(display_buf);
                            intent = 0;
                        }
                        state = STATE_WAIT_FOR_ANY;
                        break;
                    case STATE_WAIT_FOR_OBJECT:
                        if ( ((rx_bits & INFERENCE_BIT_GREEN) != 0)
                          || ((rx_bits & INFERENCE_BIT_RED) != 0) ) {
                          intent |= rx_bits;
                        } else {
                            rtos_printf("Object with no action.  Reset intent state\n");
                            ssd1306_128x32_clear_bitmap(display_buf);
                            intent = 0;
                        }
                        state = STATE_WAIT_FOR_ANY;
                        break;
                }

                /* Process intent */
                switch (intent) {
                    default:
                        break;
                    case (INFERENCE_BIT_GREEN | INFERENCE_BIT_DISPLAY):
                        green_led_on();
                        rtos_printf("Intent is GREEN ON\n");
                        ssd1306_128x32_ascii_to_bitmap("Green On", (char *)display_buf);
                        intent = 0;
                        break;
                    case (INFERENCE_BIT_GREEN | INFERENCE_BIT_CLEAR):
                        green_led_off();
                        rtos_printf("Intent is GREEN OFF\n");
                        ssd1306_128x32_ascii_to_bitmap("Green Off", (char *)display_buf);
                        intent = 0;
                        break;
                    case (INFERENCE_BIT_RED | INFERENCE_BIT_DISPLAY):
                        red_led_on();
                        rtos_printf("Intent is RED ON\n");
                        ssd1306_128x32_ascii_to_bitmap("Red On", (char *)display_buf);
                        intent = 0;
                        break;
                    case (INFERENCE_BIT_RED | INFERENCE_BIT_CLEAR):
                        red_led_off();
                        rtos_printf("Intent is RED OFF\n");
                        ssd1306_128x32_ascii_to_bitmap("Red Off", (char *)display_buf);
                        intent = 0;
                        break;
                }
            }
        }
        ssd1306_write(NULL, ssd1306_ctx, display_buf);
    }
}


// display attibs
#define MAX_ROWS 32
#define MAX_COLS 16 // (128/8)
#define TOP_MARGIN 10
#define LEFT_MARGIN 1

// font attribs
#define FONT_SIZE 1

void ssd1306_128x32_clear_bitmap(char* bitmap) {
    // Clear bitmap buffer
    memset(bitmap, 0, sizeof(char)*MAX_ROWS*MAX_COLS);
}

void ssd1306_128x32_ascii_to_bitmap(char* str_buf, char* bitmap) {
    char* cur_ptr = str_buf;
    char* cur_bmp = NULL;

    // drawing
    int draw_start_pt = (TOP_MARGIN * MAX_COLS) + LEFT_MARGIN; // Give space
    char* brush = bitmap + draw_start_pt;

    while(1) {
        // check for null terminator and edge of canvas
        int next_char_column = ((int)brush - (int)bitmap) % MAX_COLS;
        if (((char)*cur_ptr == (char)'\0') ||
            (next_char_column + FONT_SIZE >= MAX_COLS)) {
            break;
        }
        // rtos_printf("add %c to bitmap at column %d\n", (char)*cur_ptr, ((int)brush - (int)bitmap) % MAX_COLS);
        cur_bmp = font8x8_basic[(int)*(cur_ptr)];

        // draw one character
        for(int x=0; x<sizeof(font8x8_basic[0]); x++) {
            *(brush + (MAX_COLS * x)) = *(cur_bmp + x);
        }
        brush += FONT_SIZE; // move brush over
        cur_ptr++; // iterate to next character in string
    }
}

void ssd1306_128x32_add_indicator_frame_to_bitmap(char* bitmap) {
    memset(bitmap, 0xFF, MAX_COLS); // Set top row
    for(int i=1; i<MAX_ROWS; i++) { // Set middle rows
        *(bitmap + (i*MAX_COLS)) = 0x01;
        *(bitmap + (i*MAX_COLS) + (MAX_COLS - 1)) = 0x80;
    }
    memset(bitmap + (MAX_COLS*(MAX_ROWS-1)), 0xFF, MAX_COLS); // Set bottom row
}

void inference_hmi_create(unsigned priority, void *args)
{
    xTaskCreate((TaskFunction_t) inference_hmi_task,
                "inf_hmi",
                RTOS_THREAD_STACK_SIZE(inference_hmi_task),
                args,
                priority,
                NULL);
}
