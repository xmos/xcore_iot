// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#ifndef CIFAR10_MODEL_RUNNER_H_
#define CIFAR10_MODEL_RUNNER_H_

#include "drivers/sw_services/model_runner/api/model_runner.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Create a new model runner for the model content specified.
 *
 * @param[out] ctx             Model runner context
 * @param[in]  model_content   Array containing model content
 */
ModelRunnerStatus cifar10_create(model_runner_t *ctx, const uint8_t* model_content);

/** Run inference using the model runner.
 *
 * @param[in] ctx   Model runner context
 */
ModelRunnerStatus cifar10_invoke(model_runner_t *ctx);



#ifndef NDEBUG

/** Get the model profiler inference durations.
 *
 * @param[in]  ctx     Model runner context
 * @param[out] count   Number of inference durations (one per operator)
 * @param[out] times   Point to array of inference durarions
 */
void cifar10_get_profiler_times(model_runner_t *ctx, uint32_t *count,
                                     const uint32_t **times);

/** Print a summary report of profiler inference durations.
 *
 * @param[in] ctx     Model runner context
 */
void cifar10_print_profiler_summary(model_runner_t *ctx);

#endif


#ifdef __cplusplus
};
#endif

#endif  // CIFAR10_MODEL_RUNNER_H_