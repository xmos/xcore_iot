// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DRIVERS_SW_SERVICES_MODEL_RUNNER_H_
#define DRIVERS_SW_SERVICES_MODEL_RUNNER_H_

#include <stddef.h>
#include <stdint.h>

#if RTOS_FREERTOS
#include "dispatch.h"
#endif

struct model_runner_struct {
  void *hInterpreter;
  __attribute__((fptrgroup("model_runner_resolver_get_fptr_grp"))) void (
      *resolver_get_fun)(void **);
  __attribute__((fptrgroup("model_runner_profiler_get_fptr_grp"))) void (
      *profiler_get_fun)(void **);
  __attribute__((fptrgroup("model_runner_profiler_reset_fptr_grp"))) void (
      *profiler_reset_fun)(void);
  __attribute__((fptrgroup(
      "model_runner_profiler_durations_get_fptr_grp"))) void (*profiler_durations_get_fun)(uint32_t
                                                                                               *,
                                                                                           const uint32_t
                                                                                               **);
};

typedef struct model_runner_struct model_runner_t;

typedef enum ModelRunnerStatus {
  Ok = 0,
  ModelVersionError = 1,
  AllocateTensorsError = 2,
  InvokeError = 3
} ModelRunnerStatus;

#ifdef __cplusplus
extern "C" {
#endif

/** Get size of buffer needed for call to model_runner_create.
 *
 * @return   The size (in bytes)
 */
size_t model_runner_buffer_size_get();

/** Initialize the model runner global state.
 *
 * @param[in] arena        Array for scratch and activations.
 * @param[in] arena_size   Size (in bytes) of arena array
 */
void model_runner_init(uint8_t *arena, size_t arena_size);

/** Create a Dispatcher.
 *  Must be called before model_runner_allocate
 *
 * @param[in] ctx    Model runner context
 * @param[in] queue  Dispatch Queue object (for RTOS only)
 */
#if RTOS_FREERTOS
ModelRunnerStatus model_runner_dispatcher_create(model_runner_t *ctx,
                                                 dispatch_queue_t *queue);
#else
ModelRunnerStatus model_runner_dispatcher_create(model_runner_t *ctx);
#endif

/** Allocate the model runner with the specified model content.
 *
 * @param[in] ctx                Model runner context
 * @param[in] model_content      Array containing model content
 */
ModelRunnerStatus model_runner_allocate(model_runner_t *ctx,
                                        const uint8_t *model_content);

/** Get the model input buffer.
 *
 * @param[in] ctx                Model runner context
 * @param[in] model_content      Model contents
 *
 * @return    Pointer to model input buffer.
 */
int8_t *model_runner_input_buffer_get(model_runner_t *ctx);

/** Get the model input size.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Model input size (in bytes).
 */
size_t model_runner_input_size_get(model_runner_t *ctx);

/** Get the model input quantization parameters.
 *
 * @param[in]  ctx          Model runner context
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void model_runner_input_quant_get(model_runner_t *ctx, float *scale,
                                  int *zero_point);

/** Run inference using the model runner.
 *
 * @param[in] ctx   Model runner context
 */
ModelRunnerStatus model_runner_invoke(model_runner_t *ctx);

/** Get the model output buffer.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Pointer to model output buffer.
 */
int8_t *model_runner_output_buffer_get(model_runner_t *ctx);

/** Get the model output size.
 *
 * @param[in] ctx   Model runner context
 *
 * @return    Model output size (in bytes).
 */
size_t model_runner_output_size_get(model_runner_t *ctx);

/** Get the model output quantization parameters.
 *
 * @param[in]  ctx          Model runner context
 * @param[out] scale        Quantization scale
 * @param[out] zero_point   Quantization zero point
 */
void model_runner_ouput_quant_get(model_runner_t *ctx, float *scale,
                                  int *zero_point);

#ifndef NDEBUG
/** Get the profiler inference durations.
 *
 * @param[in]  ctx        Model runner context
 * @param[out] count      The number of durations in the array
 * @param[out] durations  Pointer to the durations array
 */
void model_runner_profiler_durations_get(model_runner_t *ctx, uint32_t *count,
                                         const uint32_t **durations);

/** Print a summary report of profiler inference durations.
 *
 * @param[in] ctx     Model runner context
 */
void model_runner_profiler_summary_print(model_runner_t *ctx);

#endif

#ifdef __cplusplus
};
#endif

#endif  // DRIVERS_SW_SERVICES_MODEL_RUNNER_H_
