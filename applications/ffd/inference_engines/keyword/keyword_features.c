// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "keyword_features.h"

#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

#define AUDIO_WINDOW_LENGTH_MS                                                 \
  (((float)appconfINFERENCE_FRAMES_PER_INFERENCE /                             \
    (float)appconfAUDIO_PIPELINE_SAMPLE_RATE) *                                \
   1000 * NUM_AUDIO_FRAMES_PER_FEATURES)

void initialize_features(struct FrontendState *state) {
  struct FrontendConfig config;

  FrontendFillConfigWithDefaults(&config);
  config.window.size_ms = AUDIO_WINDOW_LENGTH_MS;
  config.window.step_size_ms = AUDIO_WINDOW_LENGTH_MS;
  config.filterbank.num_channels = 16;
  config.filterbank.lower_band_limit = 125.0;
  config.filterbank.upper_band_limit = 7500.0;
  config.noise_reduction.smoothing_bits = 10;
  config.noise_reduction.even_smoothing = 0.025;
  config.noise_reduction.odd_smoothing = 0.06;
  config.noise_reduction.min_signal_remaining = 0.05;
  config.pcan_gain_control.enable_pcan = false;
  config.log_scale.enable_log = true;
  config.log_scale.scale_shift = 6;

  FrontendPopulateState(&config, state, appconfAUDIO_PIPELINE_SAMPLE_RATE);
}

void compute_features(struct FrontendOutput *output,
                             struct FrontendState *state, int16_t *audio16) {
  size_t num_samples_read;

  *output = FrontendProcessSamples(state, audio16, AUDIO_BUFFER_LENGTH,
                                   &num_samples_read);

  configASSERT(num_samples_read == AUDIO_BUFFER_LENGTH);
  for (int i = 0; i < output->size; ++i) {
    rtos_printf("%d ",
                (int)output->values[i]); // Print the feature
  }
  rtos_printf("\n");
}
