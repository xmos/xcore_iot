// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef PERSON_CLASSIFIER_H_
#define PERSON_CLASSIFIER_H_

#include <stddef.h>
#include <stdint.h>

typedef enum ModelRunnerStatus {
  Ok = 0,
  ModelVersionError = 1,
  AllocateTensorsError = 2,
  InvokeError = 3
} ModelRunnerStatus;

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the model runner global state.
 *
 * @param[in] arena        Array for scratch and activations.
 * @param[in] arena_size   Size (in bytes) of arena array
 */
void person_classifier_init(uint8_t *arena, size_t arena_size);

void person_classifier_create(void *buffer);

/** Allocate the model runner with the specified model content.
 *
 * @param[in] model_content      Array containing model content
 */
ModelRunnerStatus person_classifier_allocate(const uint8_t *model_content);

/** Get the model input buffer.
 *
 * @param[in] model_content      Model contents
 *
 * @return    Pointer to model input buffer.
 */
int8_t *person_classifier_input_buffer_get();

/** Get the model input size.
 *
 * @return    Model input size (in bytes).
 */
size_t person_classifier_input_size_get();

/** Get the model input quantization parameters.
 *
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void person_classifier_input_quant_get(float *scale,
                                  int *zero_point);

/** Run inference using the model runner.
 */
ModelRunnerStatus person_classifier_invoke();

/** Get the model output buffer.
 *
 * @return    Pointer to model output buffer.
 */
int8_t *person_classifier_output_buffer_get();

/** Get the model output size.
 *
 * @return    Model output size (in bytes).
 */
size_t person_classifier_output_size_get();

/** Get the model output quantization parameters.
 *
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void person_classifier_ouput_quant_get(float *scale,
                                  int *zero_point);

#ifndef NDEBUG

/** Print a summary report of profiler inference durations.
 */
void person_classifier_profiler_summary_print();

#endif

#ifdef __cplusplus
};
#endif

#endif // PERSON_CLASSIFIER_H_
