// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "event_groups.h"

/* Inference headers */
#include "InferenceEngine.hpp"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"

#define ASR_CHANNEL             (0)

extern "C" {
/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

#include "keyword_features.h"
#include "keyword_inference.h"
#include "keyword_model_data.h"
#include "keyword_model_labels.h"

void keyword_engine_task(keyword_engine_args_t *args);
void keyword_engine_intertile_samples_in_task(void *arg);

};

static StreamBufferHandle_t samples_to_engine_stream_buf = 0;

static xcore::RTOSInferenceEngine<4, 13> inference_engine;
static struct FrontendState frontend_state;
static struct FrontendOutput frontend_output;
static int32_t audio_recv_buf[appconfINFERENCE_FRAMES_PER_INFERENCE];
static int16_t audio16_buf[AUDIO_BUFFER_LENGTH];

static uint8_t *tensor_arena = nullptr;
constexpr size_t tensor_arena_size = 40120;

#pragma stackfunction 400  // TODO: unsure why the stack can not be computed automatically here
void keyword_engine_task(keyword_engine_args_t *args) {
  StreamBufferHandle_t input_buf = args->samples_to_engine_stream_buf;
  EventGroupHandle_t output_egrp = args->egrp_inference;

  size_t audio16_buf_index = 0;
  size_t keyword_output_index = 0;
  size_t inference_input_height = 0;
  size_t inference_input_chans = 0;
  size_t inference_input_size = 0;
  size_t inference_input_row = 0;
  int8_t *inference_input_buffer = nullptr;
  int8_t *inference_output_buffer = nullptr;
  
  /* Perform any initialization here */
  initialize_features(&frontend_state);

  tensor_arena = (uint8_t *) pvPortMalloc(tensor_arena_size);
  auto resolver = inference_engine.Initialize(tensor_arena, tensor_arena_size);
  
  // Register the model operators
  resolver->AddSoftmax();
  resolver->AddConv2D();
  resolver->AddReshape();
  resolver->AddCustom(tflite::ops::micro::xcore::rtos::Conv2D_V2_OpCode,
                      tflite::ops::micro::xcore::rtos::Register_Conv2D_V2());

  // // Load the model
  if (inference_engine.LoadModel(keyword_model_data) !=  xcore::InferenceEngineStatus::Ok) {
    rtos_printf("Invalid model provided!\n");
    vPortFree(tensor_arena);
    vTaskDelete(NULL);
  }

  inference_input_buffer = inference_engine.GetInputBuffer();
  inference_input_size = inference_engine.GetInputSize();
  inference_input_height = inference_engine.GetInputDimension(1);
  inference_input_chans = inference_engine.GetInputDimension(3);
  inference_output_buffer = inference_engine.GetOutputBuffer();

  while (1) {
    /* Receive audio frames */
    uint8_t *buf_ptr = (uint8_t *)audio_recv_buf;
    size_t buf_len = appconfINFERENCE_FRAMES_PER_INFERENCE * sizeof(int32_t);
    do {
      size_t bytes_rxed =
          xStreamBufferReceive(input_buf, buf_ptr, buf_len, portMAX_DELAY);
      buf_len -= bytes_rxed;
      buf_ptr += bytes_rxed;
    } while (buf_len > 0);

    for (int i = 0; i < appconfINFERENCE_FRAMES_PER_INFERENCE; i++) {
      /* Audio is int32, convert to int16 */
      audio16_buf[audio16_buf_index++] = (int16_t)(audio_recv_buf[i] >> 16);
      //printf("%d       %d\n", audio_recv_buf[i], audio16_buf[audio16_buf_index-1]);
      if (audio16_buf_index == AUDIO_BUFFER_LENGTH) {
        /* Compute features */
        compute_features(&frontend_output, &frontend_state, audio16_buf);

        /* Copy features to inference input tensor */
        for (int j = 0; j < frontend_output.size; j++) {
            // The feature pipeline outputs 16-bit signed integers in roughly a 0 to 670
            // We have to scale the values to the -128 to 127 signed int8 numbers
            constexpr int16_t min_feature_value = 0;
            constexpr int16_t max_feature_value = 670;
            int16_t clipped_value = frontend_output.values[i];
            if (clipped_value < min_feature_value) clipped_value = min_feature_value;
            if (clipped_value > max_feature_value) clipped_value = max_feature_value;
            inference_input_buffer[inference_input_row + inference_input_chans * j] = (255 * clipped_value) / max_feature_value - 128;
        }
        inference_input_row++;

        /* Shift the audio buffer left one shift length */
        memcpy(&audio16_buf[0], &audio16_buf[appconfINFERENCE_FRAMES_PER_INFERENCE],
               AUDIO_BUFFER_SHIFT_LENGTH * sizeof(audio16_buf[0]));
        audio16_buf_index -= AUDIO_BUFFER_SHIFT_LENGTH;

        if (inference_input_row == inference_input_height) {
          /* Last row, time to run inference */
          //rtos_printf("inference\n");
          //inference_engine.Invoke();
          //inference_engine.PrintProfilerSummary();

          /* Shift the input tensor rows "up" one shift length */
          size_t offset = FEATURE_BUFFER_SHIFT_LENGTH * inference_input_chans;
          memcpy(inference_input_buffer,
                &inference_input_buffer[offset],
                inference_input_size - offset
          );
          inference_input_row -= FEATURE_BUFFER_SHIFT_LENGTH;

          printf("outputs     %d      %d\n", (int)inference_output_buffer[0], (int)inference_output_buffer[1]);
        }
      }
    }




    /* Set output event bits */
    // switch (keyword_output_index) {
    // default:
    // case 0:
    //   xEventGroupSetBits(output_egrp, INFERENCE_BIT_A | INFERENCE_BIT_B);
    //   break;
    // case 1:
    //   xEventGroupSetBits(output_egrp, INFERENCE_BIT_A);
    //   break;
    // case 2:
    //   xEventGroupSetBits(output_egrp, INFERENCE_BIT_B);
    //   break;
    // }
    // keyword_output_index = (keyword_output_index >= 2) ? 0 : keyword_output_index + 1;
  }
}


void keyword_engine_intertile_samples_in_task(void *arg)
{
    (void) arg;

    for (;;) {
        uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
        size_t bytes_received;

        bytes_received = rtos_intertile_rx_len(
                intertile_ctx,
                appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                portMAX_DELAY);

        xassert(bytes_received == sizeof(samples));

        rtos_intertile_rx_data(
                intertile_ctx,
                samples,
                bytes_received);

        if (xStreamBufferSend(samples_to_engine_stream_buf, samples, sizeof(samples), 0) != sizeof(samples)) {
            rtos_printf("lost output samples for inference\n");
        }
    }
}


void keyword_engine_samples_send_remote(
        rtos_intertile_t *intertile_ctx,
        size_t frame_count,
        int32_t (*processed_audio_frame)[2])
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        samples[i] = (uint32_t)processed_audio_frame[i][ASR_CHANNEL];
    }

    rtos_intertile_tx(intertile_ctx,
                      appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                      samples,
                      sizeof(samples));
}

void keyword_engine_samples_send_local(
        size_t frame_count,
        int32_t (*processed_audio_frame)[2])
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    uint32_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        samples[i] = (uint32_t)(processed_audio_frame[i][ASR_CHANNEL]);
    }

    if(samples_to_engine_stream_buf != NULL) {
        if (xStreamBufferSend(samples_to_engine_stream_buf, processed_audio_frame, sizeof(samples), 0) != sizeof(samples)) {
            rtos_printf("lost local output samples for inference\n");
        }
    } else {
        rtos_printf("inference engine streambuffer not ready\n");
    }
}

void keyword_engine_task_create(uint32_t priority, keyword_engine_args_t *args)
{
    args->samples_to_engine_stream_buf = xStreamBufferCreate(
                                           appconfINFERENCE_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINFERENCE_FRAMES_PER_INFERENCE);
    samples_to_engine_stream_buf = args->samples_to_engine_stream_buf;  // TODO remove need for this to be static

    xTaskCreate((TaskFunction_t)keyword_engine_task,
                "keyword_eng",
                RTOS_THREAD_STACK_SIZE(keyword_engine_task),
                args,
                uxTaskPriorityGet(NULL),
                NULL);
}

void keyword_engine_intertile_task_create(uint32_t priority, keyword_engine_args_t *args)
{
    args->samples_to_engine_stream_buf = xStreamBufferCreate(
                                           appconfINFERENCE_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE * NUM_AUDIO_FRAMES_PER_INFERENCE,
                                           appconfINFERENCE_FRAMES_PER_INFERENCE);
    samples_to_engine_stream_buf = args->samples_to_engine_stream_buf;  // TODO remove need for this to be static

    xTaskCreate((TaskFunction_t)keyword_engine_intertile_samples_in_task,
                "inf_intertile_rx",
                RTOS_THREAD_STACK_SIZE(keyword_engine_intertile_samples_in_task),
                NULL,
                priority-1,
                NULL);

    xTaskCreate((TaskFunction_t)keyword_engine_task,
                "keyword_eng",
                RTOS_THREAD_STACK_SIZE(keyword_engine_task),
                args,
                uxTaskPriorityGet(NULL),
                NULL);
}

