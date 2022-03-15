// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xcore/hwtimer.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

/* App headers */
#include "app_conf.h"
#include "inference_engine.h"
#include "keyword_inf_eng.h"
#include "platform/driver_instances.h"

#include "tensorflow/lite/experimental/microfrontend/lib/frontend.h"
#include "tensorflow/lite/experimental/microfrontend/lib/frontend_util.h"

#define NUM_FRAMES_PER_INFERENCE (2)
#define AUDIO_BUFFER_LENGTH                                                    \
  (appconfINFERENCE_FRAMES_PER_INFERENCE * NUM_FRAMES_PER_INFERENCE)
#define AUDIO_BUFFER_STRIDE_LENGTH                                             \
  (AUDIO_BUFFER_LENGTH - appconfINFERENCE_FRAMES_PER_INFERENCE)

void keyword_engine_task(void *args) {
  StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;

  struct FrontendConfig frontend_config;
  struct FrontendState frontend_state;
  struct FrontendOutput frontend_output;
  int32_t buf[appconfINFERENCE_FRAMES_PER_INFERENCE];
  int16_t audio16[AUDIO_BUFFER_LENGTH];
  size_t audio16_index = 0;
  size_t num_frontend_samples_read;

  /* Perform any initialization here */
  FrontendFillConfigWithDefaults(&frontend_config);
  frontend_config.window.size_ms = 30;
  frontend_config.window.step_size_ms = 30;
  frontend_config.noise_reduction.smoothing_bits = 10;
  frontend_config.filterbank.num_channels = 16;
  frontend_config.filterbank.lower_band_limit = 8.0;
  frontend_config.filterbank.upper_band_limit = 450.0;
  frontend_config.noise_reduction.smoothing_bits = 10;
  frontend_config.noise_reduction.even_smoothing = 0.025;
  frontend_config.noise_reduction.odd_smoothing = 0.06;
  frontend_config.noise_reduction.min_signal_remaining = 0.05;
  frontend_config.pcan_gain_control.enable_pcan = false;
  frontend_config.log_scale.enable_log = true;
  frontend_config.log_scale.scale_shift = 6;
  FrontendPopulateState(&frontend_config, &frontend_state,
                        appconfAUDIO_PIPELINE_SAMPLE_RATE);

  // TODO: Sync frontend_config settings with the model training
  // TODO:  store input tensor of features [65*16]
  //             Need to "quantize" the features
  // TODO: call inference every TBD audio frames

  while (1) {
    /* Receive audio frames */
    uint8_t *buf_ptr = (uint8_t *)buf;
    size_t buf_len = appconfINFERENCE_FRAMES_PER_INFERENCE * sizeof(int32_t);
    do {
      size_t bytes_rxed =
          xStreamBufferReceive(input_queue, buf_ptr, buf_len, portMAX_DELAY);

      buf_len -= bytes_rxed;
      buf_ptr += bytes_rxed;
    } while (buf_len > 0);

    for (int i = 0; i < appconfINFERENCE_FRAMES_PER_INFERENCE; ++i) {
      /* Audio is int32, convert to int16 */
      audio16[audio16_index] = (int16_t)(buf[i] >> 16);
      if (audio16_index++ == AUDIO_BUFFER_LENGTH) {
        /* Compute features */
        frontend_output = FrontendProcessSamples(&frontend_state, audio16,
                                                 AUDIO_BUFFER_LENGTH,
                                                 &num_frontend_samples_read);

        configASSERT(num_frontend_samples_read == AUDIO_BUFFER_LENGTH);
        for (int i = 0; i < frontend_output.size; ++i) {
          printf("%d ",
                 (int)frontend_output.values[i]); // Print the feature
        }

        /* Shift the audio buffer left one stride*/
        memcpy(&audio16[0], &audio16[appconfINFERENCE_FRAMES_PER_INFERENCE],
               AUDIO_BUFFER_STRIDE_LENGTH * sizeof(audio16[0]));
        audio16_index = AUDIO_BUFFER_STRIDE_LENGTH;
      }
    }

    /* Perform inference here */
    // rtos_printf("inference\n");
  }
}
