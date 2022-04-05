// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "template_inf_eng.h"

int32_t inference_engine_create(uint32_t priority, void *args)
{
    (void) args;
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    template_engine_task_create(priority);
#else
    template_engine_intertile_task_create(priority);
#endif
#endif
    return 0;
}

int32_t inference_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    template_engine_samples_send_local(
            frames,
            buf);
#else
    template_engine_samples_send_remote(
            intertile_ctx,
            frames,
            buf);
#endif
#endif
    return 0;
}
