// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_mic_array.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/mic_array/mic_array_test.h"

#ifndef LIBXCORE_HWTIMER_HAS_REFERENCE_TIME
#error This test requires reference time
#endif

static const char* test_name = "get_samples_test";

#define local_printf( FMT, ... )    mic_array_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define MIC_ARRAY_TILE 1

#define EXPECTED_DURATION       MIC_ARRAY_TEST_AUDIO_SAMPLE_RATE * 100
#define EXPECTED_DURATION_MAX   (EXPECTED_DURATION * 1.01)
#define EXPECTED_DURATION_MIN   (EXPECTED_DURATION * 0.99)

MIC_ARRAY_MAIN_TEST_ATTR
static int main_test(mic_array_test_ctx_t *ctx)
{
    local_printf("Start");

    #if ON_TILE(MIC_ARRAY_TILE)
    {
        uint32_t min = 0xFFFFFFFF;
        uint32_t max = 0;
        int32_t mic_frame[MIC_ARRAY_FRAME_LEN][MIC_ARRAY_CHAN_PAIRS];
        int32_t (*mic_audio_frame)[2];

        mic_audio_frame = mic_frame;

        for (int i=0; i<MIC_ARRAY_TEST_ITERS; i++)
        {
            uint32_t start = get_reference_time();
            size_t num = rtos_mic_array_rx(ctx->mic_array_ctx, mic_audio_frame, MIC_ARRAY_FRAME_LEN, portMAX_DELAY);
            uint32_t end = get_reference_time();
            uint32_t duration = end - start;
            if (duration < min) min = duration;
            if (duration > max) max = duration;

            if (num != MIC_ARRAY_FRAME_LEN)
            {
                local_printf("Failed.  expected %u got %u", num, MIC_ARRAY_FRAME_LEN);
                return -1;
            }

            if ((duration > EXPECTED_DURATION_MAX) || (duration < EXPECTED_DURATION_MIN))
            {
                local_printf("Failed.  duration was %u", duration);
                return -1;
            }
        }
        local_printf("Min duration was %u", min);
        local_printf("Max duration was %u", max);
    }
    #endif

    local_printf("Done");
    return 0;
}

void register_get_samples_test(mic_array_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
