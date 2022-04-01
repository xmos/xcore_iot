// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "dsp_filters.h"
#include "string.h"

#define EXP -31

void apply_fir(xs3_filter_fir_s32_t * filter, int32_t DWORD_ALIGNED data[FILTER_FRAME_LENGTH]){

    bfp_s32_t data_bfp;

    for(int v = 0; v < FILTER_FRAME_LENGTH; v++){
        data[v] = xs3_filter_fir_s32(filter, data[v]);
    }
    bfp_s32_init(&data_bfp, data, EXP, FILTER_FRAME_LENGTH, 1);
    // normalising the exponent
    bfp_s32_use_exponent(&data_bfp, EXP);
    // clear the filter data to be able to reuse the filter structure
    memset(filter->state, 0, FIR_NUM_TAPS * sizeof(int32_t));
}

void apply_biquad(xs3_biquad_filter_s32_t * filter, int32_t DWORD_ALIGNED data[FILTER_FRAME_LENGTH]){

    bfp_s32_t bfp;
    bfp_s32_init(&bfp, data, EXP, FILTER_FRAME_LENGTH, 1);
    // shifting one bit right to match the coeficients format 
    // the output will be in Q30, so we need to shift it one bit left
    bfp_s32_use_exponent(&bfp, BIQUAD_FILT_EXP);

    for(int v = 0; v < FILTER_FRAME_LENGTH; v++){
        data[v] = xs3_filter_biquad_s32(filter, data[v]);
    }
    // using bfp because it has a saturation logic
    bfp_s32_use_exponent(&bfp, EXP);

    // clear the filter data to be able to reuse the filter structure
    memset(filter->state, 0, sizeof(filter->state));
}
