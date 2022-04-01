// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef EQUALISER_H
#define EQUALISER_H

#include <xs3_math.h>
#include <dsp_filters.h>

#define NUM_BANDS 28
#define NUM_STRUCTS ((NUM_BANDS + BIQUADS_IN_STRUCT - 1) / (BIQUADS_IN_STRUCT))

typedef struct {

    xs3_biquad_filter_s32_t filter[NUM_STRUCTS];
    float band_dBgain[NUM_BANDS];
    float master_dBgain;

} equaliser_t;

void eq_init(equaliser_t * eq);

void eq_get_biquads(equaliser_t * eq);

void eq_process_frame(equaliser_t * eq, int32_t frame[FILTER_FRAME_LENGTH]);

#endif
