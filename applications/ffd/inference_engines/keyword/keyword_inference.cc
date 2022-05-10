// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "stream_buffer.h"
#include "task.h"

/* Inference headers */
#include "InferenceEngine.hpp"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"

#define ASR_CHANNEL (0)

extern "C" {
/* STD headers */
#include <platform.h>
#include <xcore/hwtimer.h>
#include <xs1.h>

#include "keyword_features.h"
#include "keyword_inference.h"
#include "keyword_model_data.h"
#include "keyword_model_labels.h"

void keyword_engine_task(keyword_engine_args_t *args);
void keyword_engine_intertile_samples_in_task(void *arg);
};

static StreamBufferHandle_t samples_to_engine_stream_buf = 0;

static xcore::rtos::InferenceEngine<4, 13> inference_engine;
static struct FrontendState frontend_state;
static struct FrontendOutput frontend_output;
static int32_t audio32_recv_buf[appconfINFERENCE_FRAMES_PER_INFERENCE];
static int16_t audio16_recv_buf[appconfINFERENCE_FRAMES_PER_INFERENCE];

static uint8_t *tensor_arena = nullptr;
constexpr size_t tensor_arena_size = 42000;

// TODO: unsure why the stack can not be computed automatically here
#pragma stackfunction 400
void keyword_engine_task(keyword_engine_args_t *args) {
  StreamBufferHandle_t input_buf = args->samples_to_engine_stream_buf;
  EventGroupHandle_t output_egrp = args->egrp_inference;

  // size_t keyword_output_index = 0;
  size_t num_samples_read = 0;
  size_t input_height = 0;
  size_t input_width = 0;
  size_t input_chans = 0;
  size_t input_size = 0;
  int8_t *inference_input_buffer = nullptr;
  size_t output_size = 0;
  int8_t *output_buffer = nullptr;
  int8_t *feature_input_buffer = nullptr;
  size_t feature_input_row = 0;
  // xcore::rtos::QuantizationParams input_quant;
  xcore::rtos::QuantizationParams output_quant;

  /* Perform any initialization here */
  initialize_features(&frontend_state);

  tensor_arena = (uint8_t *)pvPortMalloc(tensor_arena_size);

  // Register the model operators
  auto resolver = inference_engine.Initialize(tensor_arena, tensor_arena_size);
  resolver->AddSoftmax();
  resolver->AddConv2D();
  resolver->AddReshape();
  resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_V2_OpCode,
                      tflite::ops::micro::xcore::rtos::Register_Conv2D_V2());

  // Load the model
  if (inference_engine.LoadModel(keyword_model_data) !=
      xcore::rtos::InferenceEngineStatus::Ok) {
    rtos_printf("Invalid model provided!\n");
    vPortFree(tensor_arena);
    vPortFree(feature_input_buffer);
    vTaskDelete(NULL);
  }

  inference_input_buffer = inference_engine.GetInputBuffer();
  input_height = inference_engine.GetInputDimension(1);
  input_width = inference_engine.GetInputDimension(2);
  input_chans = inference_engine.GetInputDimension(3);
  input_size = inference_engine.GetInputSize();
  // input_quant = inference_engine.GetInputQuantization();

  output_buffer = inference_engine.GetOutputBuffer();
  output_size = inference_engine.GetOutputSize();
  output_quant = inference_engine.GetOutputQuantization();

  feature_input_buffer = (int8_t *)pvPortMalloc(input_size);

  while (1) {
    /* Receive audio frames */
    uint8_t *buf_ptr = (uint8_t *)audio32_recv_buf;
    size_t buf_len = appconfINFERENCE_FRAMES_PER_INFERENCE * sizeof(int32_t);

    do {
      size_t bytes_rxed =
          xStreamBufferReceive(input_buf, buf_ptr, buf_len, portMAX_DELAY);
      buf_len -= bytes_rxed;
      buf_ptr += bytes_rxed;
    } while (buf_len > 0);

    for (int i = 0; i < appconfINFERENCE_FRAMES_PER_INFERENCE; i++) {
      /* Audio is int32, convert to int16 */
      audio16_recv_buf[i] = (int16_t)(audio32_recv_buf[i] >> 16);
    }

    num_samples_read =
        compute_features(&frontend_output, &frontend_state, audio16_recv_buf);

    if (frontend_output.size == FEATURE_COUNT) {
      /* Copy features to inference input tensor */
      for (int i = 0; i < FEATURE_COUNT; i++) {
        // int8_t quant_value = (int8_t)((float)frontend_output.values[i] /
        // input_quant.scale + input_quant.zero_point);
        size_t input_index =
            feature_input_row * input_width * input_chans + input_chans * i;
        constexpr int32_t value_scale = 256;
        constexpr int32_t value_div =
            static_cast<int32_t>((25.6f * 26.0f) + 0.5f);
        int32_t quant_value =
            ((frontend_output.values[i] * value_scale) + (value_div / 2)) /
            value_div;
        quant_value -= 128;
        if (quant_value < -128) {
          quant_value = -128;
        }
        if (quant_value > 127) {
          quant_value = 127;
        }
        feature_input_buffer[input_index] = quant_value;
      }

      feature_input_row++;

      if (feature_input_row == input_height) {
        /* Last row, time to run inference */
        memcpy(inference_input_buffer, feature_input_buffer, input_size);
        inference_engine.Invoke();
        for (int i = 0; i < output_size; i++) {
          float prob = (float)(output_buffer[i] - output_quant.zero_point) *
                       output_quant.scale * 100.0;
          if (prob >= 80) {
            // rtos_printf("recognized %s (%d%%)\n", keyword_model_labels[i],
            // (int)prob);
            xEventGroupSetBits(output_egrp, (1 << i));
          }
        }

        /* Shift the input tensor rows "up" one shift length */
        size_t offset = FEATURE_INPUT_BUFFER_SHIFT * input_chans * input_width;
        memmove(feature_input_buffer, feature_input_buffer + offset,
                input_size - offset);
        feature_input_row -= FEATURE_INPUT_BUFFER_SHIFT;
      }
    }
  }
}

void keyword_engine_intertile_samples_in_task(void *arg) {
  (void)arg;

  for (;;) {
    int32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    size_t bytes_received;

    bytes_received = rtos_intertile_rx_len(
        intertile_ctx, appconfINTENT_MODEL_RUNNER_SAMPLES_PORT, portMAX_DELAY);

    xassert(bytes_received == sizeof(samples));

    rtos_intertile_rx_data(intertile_ctx, samples, bytes_received);

    if (xStreamBufferSend(samples_to_engine_stream_buf, samples,
                          sizeof(samples), 0) != sizeof(samples)) {
      rtos_printf("lost output samples for inference\n");
    }
  }
}

void keyword_engine_samples_send_remote(rtos_intertile_t *intertile_ctx,
                                        size_t frame_count,
                                        int32_t *processed_audio_frame) {
  configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

  rtos_intertile_tx(intertile_ctx, appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                    processed_audio_frame, sizeof(int32_t) * frame_count);
}

void keyword_engine_samples_send_local(size_t frame_count,
                                       int32_t *processed_audio_frame) {
  configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

  if (samples_to_engine_stream_buf != NULL) {
    size_t bytes_to_send = sizeof(int32_t) * frame_count;
    if (xStreamBufferSend(samples_to_engine_stream_buf, processed_audio_frame,
                          bytes_to_send, 0) != bytes_to_send) {
      rtos_printf("lost local output samples for inference\n");
    }
  } else {
    rtos_printf("inference engine streambuffer not ready\n");
  }
}

void keyword_engine_task_create(uint32_t priority,
                                keyword_engine_args_t *args) {
  args->samples_to_engine_stream_buf = xStreamBufferCreate(
      appconfINFERENCE_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
      appconfINFERENCE_FRAMES_PER_INFERENCE);
  samples_to_engine_stream_buf =
      args->samples_to_engine_stream_buf; // TODO remove need for this to be
                                          // static

  xTaskCreate((TaskFunction_t)keyword_engine_task, "keyword_eng",
              RTOS_THREAD_STACK_SIZE(keyword_engine_task), args,
              uxTaskPriorityGet(NULL), NULL);
}

void keyword_engine_intertile_task_create(uint32_t priority,
                                          keyword_engine_args_t *args) {
  args->samples_to_engine_stream_buf = xStreamBufferCreate(
      appconfINFERENCE_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE *
          NUM_AUDIO_FRAMES_PER_FEATURES,
      appconfINFERENCE_FRAMES_PER_INFERENCE);
  samples_to_engine_stream_buf =
      args->samples_to_engine_stream_buf; // TODO remove need for this to be
                                          // static

  xTaskCreate((TaskFunction_t)keyword_engine_intertile_samples_in_task,
              "inf_intertile_rx",
              RTOS_THREAD_STACK_SIZE(keyword_engine_intertile_samples_in_task),
              NULL, priority - 1, NULL);

  xTaskCreate((TaskFunction_t)keyword_engine_task, "keyword_eng",
              RTOS_THREAD_STACK_SIZE(keyword_engine_task), args,
              uxTaskPriorityGet(NULL), NULL);
}
