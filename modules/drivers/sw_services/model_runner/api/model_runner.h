// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef DRIVERS_SW_SERVICES_MODEL_RUNNER_H_
#define DRIVERS_SW_SERVICES_MODEL_RUNNER_H_

#include <stddef.h>
#include <stdint.h>

typedef struct model_runner_struct model_runner_t;

struct model_runner_struct {
  void *handle;
};

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
void model_runner_init(uint8_t *arena, int arena_size);

/** Create a new model runner for the model content specified.
 *
 * @param[out] ctx             Model runner context
 * @param[in]  resolver        MicroMutableOpResolver (cast to void*)
 * @param[in]  model_content   Array containing model content
 */
ModelRunnerStatus model_runner_create(model_runner_t *ctx, void *resolver,
                                      const uint8_t *model_content);

/** Get the model input.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Pointer to model input buffer.
 */
int8_t *model_runner_get_input(model_runner_t *ctx);

/** Get the model input size.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Model input size (in bytes).
 */
size_t model_runner_get_input_size(model_runner_t *ctx);

/** Get the model input quantization parameters.
 *
 * @param[in]  ctx          Model runner context
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void model_runner_get_input_quant(model_runner_t *ctx, float *scale,
                                  int *zero_point);

/** Run inference using the model runner.
 *
 * @param[in] ctx   Model runner context
 */
ModelRunnerStatus model_runner_invoke(model_runner_t *ctx);

/** Get the model output.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Pointer to model output buffer.
 */
int8_t *model_runner_get_output(model_runner_t *ctx);

/** Get the model output size.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Model output size (in bytes).
 */
size_t model_runner_get_output_size(model_runner_t *ctx);

/** Get the model output quantization parameters.
 *
 * @param[in]  ctx          Model runner context
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void model_runner_get_ouput_quant(model_runner_t *ctx, float *scale,
                                  int *zero_point);

#ifndef NDEBUG

/** Get the model profiler inference durations.
 *
 * @param[in]  ctx          Model runner context
 * @param[out] count   Number of inference durations (one per operator)
 * @param[out] times   Point to array of inference durarions
 */
void model_runner_get_profiler_times(model_runner_t *ctx, uint32_t *count,
                                     const uint32_t **times);

/** Print a summary report of profiler inference durations.
 *
 * @param[in]  ctx          Model runner context
 */
void model_runner_print_profiler_summary(model_runner_t *ctx);

#endif

#ifdef __cplusplus
};
#endif

#endif  // DRIVERS_SW_SERVICES_MODEL_RUNNER_H_