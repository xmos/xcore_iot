// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "keyword_features.h"

#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

#define AUDIO_WINDOW_LENGTH_MS                                                 \
  (((float)appconfINFERENCE_FRAMES_PER_INFERENCE /                             \
    (float)appconfAUDIO_PIPELINE_SAMPLE_RATE) *                                \
   1000 * NUM_AUDIO_FRAMES_PER_FEATURES)

#define AUDIO_WINDOW_STEP_MS (AUDIO_WINDOW_LENGTH_MS / 2)

void initialize_features(struct FrontendState *state) {
  struct FrontendConfig config;

  FrontendFillConfigWithDefaults(&config);

  config.window.size_ms = (int)AUDIO_WINDOW_LENGTH_MS;
  config.window.step_size_ms = (int)AUDIO_WINDOW_STEP_MS;
  config.filterbank.num_channels = FEATURE_COUNT;
  config.pcan_gain_control.enable_pcan = false;

  FrontendPopulateState(&config, state, appconfAUDIO_PIPELINE_SAMPLE_RATE);
}

size_t compute_features(struct FrontendOutput *output,
                        struct FrontendState *state, int16_t *audio16) {
  size_t num_samples_read;

  *output = FrontendProcessSamples(state, audio16, appconfINFERENCE_FRAMES_PER_INFERENCE,
                                   &num_samples_read);

  // for (int i = 0; i < output->size; ++i) {
  //   rtos_printf("%d ",
  //               (int)output->values[i]); // Print the feature
  // }
  // rtos_printf("\n");
  return num_samples_read;
}
