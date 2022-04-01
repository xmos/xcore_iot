// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef SDK_DSP_FILTERS_
#define SDK_DSP_FILTERS_

#include "bfp_math.h"

#define FIR_NUM_TAPS 128

#define BIQUADS_IN_STRUCT 8
#define COEF_PER_BIQUAD 5
#define BIQUAD_FILT_EXP -30

#define FILTER_FRAME_LENGTH 240


/**
 * @brief Applies IIR biquad filtering on the data frame
 * 
 * @param[inout] filter filter structure which keeps the filter coefficients and data
 * @param[inout] data array of the frame data on which to perform the IIR filtering
 */
void apply_biquad(xs3_biquad_filter_s32_t * filter, int32_t DWORD_ALIGNED data[FILTER_FRAME_LENGTH]);

/**
 * @brief Applies FIR filtering on the data frame
 * 
 * @param[inout] filter filter structure which keeps coefficients and filter data
 * @param[inout] data array of the frame data on which to perform the FIR filtering
 */
void apply_fir(xs3_filter_fir_s32_t * filter, int32_t DWORD_ALIGNED data[FILTER_FRAME_LENGTH]);

#endif
