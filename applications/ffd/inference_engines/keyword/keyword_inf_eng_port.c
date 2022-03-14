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
#include "keyword_inf_eng.h"

int32_t inference_engine_create(void *args)
{
    uint32_t prio = (uint32_t)args;
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    keyword_engine_task_create(prio);
#else
    keyword_engine_intertile_task_create(prio);
#endif
#endif
    return 0;
}

int32_t inference_engine_sample_push(uint8_t *buf, size_t bytes)
{
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    keyword_engine_samples_send_local(
            bytes,
            (int32_t(*)[2])buf);
#else
    keyword_engine_samples_send_remote(
            intertile_ctx,
            bytes,
            (int32_t(*)[2])buf);
#endif
#endif
    return 0;
}
