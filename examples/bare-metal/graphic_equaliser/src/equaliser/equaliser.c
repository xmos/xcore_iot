// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "equaliser.h"
#include <xs3_vpu_scalar_ops.h>
#include <math.h>
#include <string.h>

#define Q float_to_float_s32(4.318477)
#define SAMPLE_RATE (float_s32_t){16000, 0}
#define PI float_to_float_s32(3.14159)
#define NORM_EXP -31

// array that stores center frequencies with the third octave spacing
const float center_freq[NUM_BANDS] = {
    12.5, 16.0, 20.0, 25.0, 31.5,
    40.0, 50.0, 63.0, 80.0, 100.0,
    125.0, 160.0, 200.0, 250.0, 315.0,
    400.0, 500.0, 630.0, 800.0, 1000.0,
    1250.0, 1600.0, 2000.0, 2500.0, 3150.0,
    4000.0, 5000.0, 6300.0
};

// returns the fixed point value with applyed filter exponent
int32_t eq_float_use_filt_exp(const float_s32_t num){
    right_shift_t delta = BIQUAD_FILT_EXP - num.exp;
    return vlashr32(num.mant, delta);
}

// calculates amplidude gain
// A = 10^(gain_fl/20)
float_s32_t eq_get_A(const float gain_fl){
    float_s32_t gain = float_to_float_s32(gain_fl);
    float_s32_t power = float_s32_div(gain, (float_s32_t){20, 0});
    float A_fl = powf(10.0, float_s32_to_float(power));
    return float_to_float_s32(A_fl);
}

// K = tan((pi * fr_fl) / sample_rate)
float_s32_t eq_get_K(const float fr_fl){
    float_s32_t fr = float_to_float_s32(fr_fl);
    float_s32_t factor = float_s32_div(fr, SAMPLE_RATE);
    factor = float_s32_mul(factor, PI);
    float K_fl = tanf(float_s32_to_float(factor));
    return float_to_float_s32(K_fl);
}

// calculates coefficients for a single biquad filter
// all the formulas for the coefficients are taken from:
// https://www.dsprelated.com/showcode/169.php
void eq_get_coefs(fixed_s32_t coefs[COEF_PER_BIQUAD], float_s32_t A, const float_s32_t K){
    
    unsigned pos_gain = float_s32_gt(A, (float_s32_t){1, 0});

    if(!pos_gain){
        A = float_s32_div((float_s32_t){1,0}, A); // invert gain if a cut
    }
    // calculating common factors
    const float_s32_t factor_wout_V0 = float_s32_div(K, Q); // (1/Q)*K
    const float_s32_t factor_with_V0 = float_s32_mul(factor_wout_V0, A); // (V0/Q)*K
    const float_s32_t KK = float_s32_mul(K, K); // K^2
    const float_s32_t one_plus_KK = float_s32_add((float_s32_t){1,0}, KK);// 1 + K^2

    float_s32_t  den;
    if(pos_gain){den = float_s32_add(factor_wout_V0, one_plus_KK);}
    else {den = float_s32_add(factor_with_V0, one_plus_KK);}

    float_s32_t temp;
    if(pos_gain){temp = float_s32_add(one_plus_KK,factor_with_V0);}
    else{temp = float_s32_add(one_plus_KK, factor_wout_V0);}
    temp = float_s32_div(temp, den);
    coefs[0] = eq_float_use_filt_exp(temp); // b0

    temp = float_s32_sub(KK, (float_s32_t){1,0});
    temp = float_s32_mul(temp, (float_s32_t){2,0});
    temp = float_s32_div(temp, den);
    coefs[1] = eq_float_use_filt_exp(temp); // b1

    if(pos_gain){temp = float_s32_sub(one_plus_KK,factor_with_V0);}
    else{temp = float_s32_sub(one_plus_KK, factor_wout_V0);}
    temp = float_s32_div(temp, den);
    coefs[2] = eq_float_use_filt_exp(temp); // b2

    // lib_xs3_math does not rectify the 'a.' coefficients
    coefs[3] = -coefs[1]; // a1

    if(!pos_gain){temp = float_s32_sub(one_plus_KK,factor_with_V0);}
    else{temp = float_s32_sub(one_plus_KK, factor_wout_V0);}
    temp = float_s32_div(temp, den);
    coefs[4] = -eq_float_use_filt_exp(temp); // a2
}

// fills the generated coefficients into the filter structs
void eq_get_biquads(equaliser_t * eq){
    for(int i = 0; i < NUM_BANDS; i++){
        float_s32_t A = eq_get_A(eq->band_dBgain[i]);
        float_s32_t K = eq_get_K(center_freq[i]);
        int32_t coef[COEF_PER_BIQUAD] = {0};
        int8_t struct_id = i / BIQUADS_IN_STRUCT;
        int8_t biquad_id = i % BIQUADS_IN_STRUCT;
        eq_get_coefs(coef, A, K);
        for(int j = 0; j < COEF_PER_BIQUAD; j++){
            eq->filter[struct_id].coef[j][biquad_id] = coef[j];
        }
        eq->filter[struct_id].biquad_count ++;
    }
}

// applyes biquads and gain to the frame 
void eq_process_frame(equaliser_t * eq, int32_t frame[FILTER_FRAME_LENGTH]){
    for(int v = 0; v < NUM_STRUCTS; v++){
        apply_biquad(&eq->filter[v], frame);
    }
    bfp_s32_t frame_bfp;
    bfp_s32_init(&frame_bfp, frame, NORM_EXP, FILTER_FRAME_LENGTH, 1);
    bfp_s32_scale(&frame_bfp, &frame_bfp, eq_get_A(eq->master_dBgain));
    bfp_s32_use_exponent(&frame_bfp, NORM_EXP);
}

// initialises the eq structure, can be used to reset it
void eq_init(equaliser_t * eq){
    memset(eq, 0, sizeof(equaliser_t));
}
