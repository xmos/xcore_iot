// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef AEC_CONFIG_H_
#define AEC_CONFIG_H_

#define AEC_MAX_Y_CHANNELS        (2)
#define AEC_MAX_X_CHANNELS        (2)
#define AEC_MAIN_FILTER_PHASES    (10)
#define AEC_SHADOW_FILTER_PHASES  (5)

void aec_process_frame_1thread(
        aec_state_t *main_state,
        aec_state_t *shadow_state,
        int32_t (*output_main)[AEC_FRAME_ADVANCE],
        int32_t (*output_shadow)[AEC_FRAME_ADVANCE],
        const int32_t (*y_data)[AEC_FRAME_ADVANCE],
        const int32_t (*x_data)[AEC_FRAME_ADVANCE]);

#endif /* AEC_CONFIG_H_ */
