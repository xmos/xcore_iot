// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef KEYWORD_FEATURES_H_
#define KEYWORD_FEATURES_H_

#include "app_conf.h"

#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"

#define NUM_AUDIO_FRAMES_PER_FEATURES    (2)
#define AUDIO_BUFFER_LENGTH              (appconfINFERENCE_FRAMES_PER_INFERENCE * NUM_AUDIO_FRAMES_PER_FEATURES)
#define AUDIO_BUFFER_SHIFT_LENGTH        (AUDIO_BUFFER_LENGTH - appconfINFERENCE_FRAMES_PER_INFERENCE)

#define NUM_AUDIO_FRAMES_PER_INFERENCE   (4)
#define FEATURE_BUFFER_SHIFT_LENGTH      (NUM_AUDIO_FRAMES_PER_INFERENCE)

#ifdef __cplusplus
extern "C" {
#endif

void initialize_features(struct FrontendState *state);
void compute_features(struct FrontendOutput *output,
                      struct FrontendState *state, int16_t *audio16);

#ifdef __cplusplus
};
#endif

#endif /* KEYWORD_FEATURES_H_ */