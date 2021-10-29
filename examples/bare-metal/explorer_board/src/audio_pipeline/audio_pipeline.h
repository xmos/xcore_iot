// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef AUDIO_PIPELINE_H_
#define AUDIO_PIPELINE_H_

#include <xcore/parallel.h>
#include <xcore/chanend.h>

DECLARE_JOB(ap_stage_a, (chanend_t, chanend_t));
DECLARE_JOB(ap_stage_b, (chanend_t, chanend_t, chanend_t));
DECLARE_JOB(ap_stage_c, (chanend_t, chanend_t, chanend_t));

#endif /* AUDIO_PIPELINE_H_ */
